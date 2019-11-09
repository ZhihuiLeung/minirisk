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

string my_date_transorm(string date_str)
{
    auto index_one = date_str.find('-', 0);
    auto index_two = date_str.find('-', index_one+1);
    
    return std::to_string(std::stoi(date_str.substr(0, index_one))) + '-' + std::to_string(std::stoi(date_str.substr(index_one+1,index_two))) + '-' + date_str.substr(index_two+1,date_str.size());
}
std::pair<double,string> PricerPayment::price(Market& mkt, const FixingDataServer& fds) const
{
    ptr_disc_curve_t disc = mkt.get_discount_curve(m_ir_curve);

    auto df_result = disc->df(m_dt); // this throws an exception if m_dt<today
    auto df = df_result.first;

    if(df>1)
    {
        if(df_result.second == 1) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Curve "+m_ir_curve+" DF not available before anchor date "+my_date_transorm(mkt.get_today().to_string())+", requested "+my_date_transorm(Date(mkt.get_today().get_serial()+unsigned(df)).to_string()));
        else if(df_result.second == 2) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Curve "+m_ir_curve+" DF not available beyond last tenor date "+my_date_transorm(Date(mkt.get_today().get_serial()+unsigned(df)).to_string())+", requested "+my_date_transorm(m_dt.to_string()));
    }
    
    // This PV is expressed in m_ccy. It must be converted in USD.

    if(m_fx_ccy != mkt.get_baseccy())
    {
        
        if (m_fx_ccy != "USD")
        {
            ptr_fx_spot_curve_t fx = mkt.get_fx_spot_curve(fx_spot_prefix+m_fx_ccy);
            // std::map<string, double> temp_map = mkt.get_fx_spot(m_fx_ccy);
            // auto temp = temp_map.find("FX.SPOT."+m_fx_ccy.substr(m_fx_ccy.length()-3, 3));
            // df *= (temp->second);
            auto temp = fx->get_spot(fx_spot_prefix+mkt.get_baseccy());
            df *= temp;
            // df *= fx->get_spot(fx_spot_prefix+mkt.get_baseccy());
        } else {
            ptr_fx_spot_curve_t fx = mkt.get_fx_spot_curve(fx_spot_prefix+mkt.get_baseccy());

            df /= fx->get_spot(fx_spot_prefix+"USD");
        } 
    }
    // else
    // {
    //     std::string temp_str = "FX.SPOT." + m_fx_ccy.substr(m_fx_ccy.length()-3, 3) + ".USD";
    //     std::map<string, double> temp_map = mkt.get_fx_spot(temp_str);
    //     auto temp = temp_map.find("FX.SPOT."+m_fx_ccy.substr(m_fx_ccy.length()-3, 3));
    //     df /= (temp->second);
    // }
    // std::cout << m_amt * df << std::endl;
    return std::make_pair(m_amt * df, "");
}

} // namespace minirisk


