#include "PricerForward.h"

#include <cmath>

#include "Global.h"
#include "Macros.h"

namespace minirisk {

PricerForward::PricerForward(const TradeFXForward& trd, const std::string& base_ccy)
    : m_amt(trd.quantity()),
      m_strike(trd.get_strike()),
      m_fwd_base_ccy(trd.get_base_ccy()),
      m_fwd_quote_ccy(trd.get_quote_ccy()),
      m_fwd_fixing_date(trd.get_fixing_date()),
      m_fwd_settlement_date(trd.get_settlement_date()),
      m_base_ccy(base_ccy) {}

std::pair<double, string> PricerForward::price(Market& mkt, const FixingDataServer& fds) const {
    std::string m_ir_curve = ir_curve_discount_name(m_fwd_quote_ccy);
    ptr_disc_curve_t disc = mkt.get_discount_curve(m_ir_curve);

    double df = 0;
    double fx_spot = 1;
    double s_fx_spot = 1;
    double fwd_price = 0;
    // discount factor
    auto df_result = disc->df(m_fwd_settlement_date); // this throws an exception if m_dt<today
    df = df_result.first;

    // T_0 > T_2
    if(df>1)
    {
        if(df_result.second == 1) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Curve "+m_ir_curve+", DF not available before anchor date "+my_date_transform(mkt.get_today().to_string())+", requested "+my_date_transform(Date(mkt.get_today().get_serial()+unsigned(df)).to_string()));
        else if(df_result.second == 2) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Curve "+m_ir_curve+", DF not available beyond last tenor date "+my_date_transform(Date(mkt.get_today().get_serial()+unsigned(df)).to_string())+", requested "+my_date_transform(m_fwd_settlement_date.to_string()));
    }
    
    // fx_spot
    // This PV is expressed in m_ccy. It must be converted in USD.
    if(m_fwd_quote_ccy != mkt.get_baseccy())
    {
        if (m_fwd_quote_ccy != "USD")
        {
            ptr_fx_spot_curve_t fx = mkt.get_fx_spot_curve(fx_spot_prefix+m_fwd_quote_ccy);
            fx_spot = fx->get_spot(fx_spot_prefix+mkt.get_baseccy());
        } else {
            ptr_fx_spot_curve_t fx = mkt.get_fx_spot_curve(fx_spot_prefix+mkt.get_baseccy());
            fx_spot = 1 / fx->get_spot(fx_spot_prefix+"USD");
        }
    }

    // fwd_price
    if(!(fds.is_empty()))
    {
        const auto fixing_name = fx_spot_name(m_fwd_base_ccy, m_fwd_quote_ccy);
        // T_1 <= T_0 <= T_2
        if(mkt.get_today() >= m_fwd_fixing_date)
        {
            // T_1 = T_0 and fixing data is available
            auto temp_pair = fds.get(fixing_name, m_fwd_fixing_date);
            if(temp_pair.second == "")
            {
                fwd_price = temp_pair.first;
                // T_1 < T_0
            } else if(mkt.get_today() > m_fwd_fixing_date) {
                return std::make_pair(std::numeric_limits<double>::quiet_NaN(), temp_pair.second);
            }
        }
    }
        //  T_1 > T_0 or T_1 = T_0 and fixing data is not available
    if(fwd_price == 0) {
        if(m_fwd_base_ccy != m_fwd_quote_ccy)
        {
            if (m_fwd_base_ccy != "USD")
            {
                ptr_fx_spot_curve_t fx = mkt.get_fx_spot_curve(fx_spot_prefix+m_fwd_base_ccy);
                s_fx_spot = fx->get_spot(fx_spot_prefix+m_fwd_quote_ccy);
            } else {
                ptr_fx_spot_curve_t fx = mkt.get_fx_spot_curve(fx_spot_prefix+m_fwd_quote_ccy);

                s_fx_spot = 1 / fx->get_spot(fx_spot_prefix+"USD");
            }
        }
        // get df of m_fwd_base_ccy
        std::string m_ir_curve_temp = ir_curve_discount_name(m_fwd_base_ccy);
        ptr_disc_curve_t disc_temp = mkt.get_discount_curve(m_ir_curve_temp);
        auto df_result_temp = disc_temp->df(m_fwd_fixing_date); // this throws an exception if m_dt<today
        auto df_one = df_result_temp.first;

        // get df of m_fwd_quote_ccy
        m_ir_curve_temp = ir_curve_discount_name(m_fwd_quote_ccy);
        disc_temp = mkt.get_discount_curve(m_ir_curve_temp);
        df_result_temp = disc_temp->df(m_fwd_fixing_date); // this throws an exception if m_dt<today
        auto df_two = df_result_temp.first;

        fwd_price = s_fx_spot * df_one / df_two;
    }

    if(fwd_price == 0)
    {
        if(m_fwd_base_ccy != "USD")
        {
            ptr_fx_spot_curve_t temp_fx_curve = mkt.get_fx_spot_curve(fx_spot_prefix + m_fwd_base_ccy);
            fwd_price = temp_fx_curve->get_spot(fx_spot_prefix+m_fwd_quote_ccy);
        } else {
            ptr_fx_spot_curve_t temp_fx_curve = mkt.get_fx_spot_curve(fx_spot_prefix + m_fwd_quote_ccy);
            fwd_price = 1 / temp_fx_curve->get_spot(fx_spot_prefix+m_fwd_base_ccy);
        }
    }

    MYASSERT(!(fwd_price == 0), "FX forward or fixing not available " 
    << m_fwd_base_ccy << m_fwd_quote_ccy << " for " << m_fwd_fixing_date.to_string());
    MYASSERT(!(df == 0), "Disc factor not available " 
    << m_fwd_base_ccy << m_fwd_quote_ccy << " for " << m_fwd_settlement_date.to_string());
    
    auto result = m_amt * df * (fwd_price - m_strike)* fx_spot;
    return std::make_pair(result, "");
}

} // namespace minirisk