#include "PricerPayment.h"
#include "TradePayment.h"
#include "CurveDiscount.h"
#include <map>
namespace minirisk {

PricerPayment::PricerPayment(const TradePayment& trd, const std::string& baseccy)
    : m_amt(trd.quantity())
    , m_dt(trd.delivery_date())
    , m_ir_curve(ir_curve_discount_name(trd.ccy()))
    , m_fx_ccy(trd.ccy())
{
}
std::pair<double,string> PricerPayment::price(Market& mkt, const FixingDataServer& fds) const
{
    ptr_disc_curve_t disc = mkt.get_discount_curve(m_ir_curve);
    double fx_spot = 1;
    
    auto df_result = disc->df(m_dt); // this throws an exception if m_dt<today
    auto df = df_result.first;

    if(df>1)
    {
        if(df_result.second == 1) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Curve "+m_ir_curve+", DF not available before anchor date "+my_date_transform(mkt.get_today().to_string())+", requested "+my_date_transform(Date(mkt.get_today().get_serial()+unsigned(df)).to_string()));
        else if(df_result.second == 2) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Curve "+m_ir_curve+", DF not available beyond last tenor date "+my_date_transform(Date(mkt.get_today().get_serial()+unsigned(df)).to_string())+", requested "+my_date_transform(m_dt.to_string()));
    }
    
    // This PV is expressed in m_ccy. It must be converted in USD.

    if(m_fx_ccy != mkt.get_baseccy())
    {
        
        if (m_fx_ccy != "USD")
        {
            ptr_fx_spot_curve_t fx = mkt.get_fx_spot_curve(fx_spot_prefix+m_fx_ccy);
            fx_spot = fx->get_spot(fx_spot_prefix+mkt.get_baseccy());
        } else {
            ptr_fx_spot_curve_t fx = mkt.get_fx_spot_curve(fx_spot_prefix+mkt.get_baseccy());

            fx_spot = 1 / fx->get_spot(fx_spot_prefix+"USD");
        } 
    }
    auto result = m_amt * df * fx_spot;
    return std::make_pair(result, "");
}

} // namespace minirisk


