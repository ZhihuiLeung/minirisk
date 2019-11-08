#include "PricerPayment.h"
#include "TradePayment.h"
#include "CurveDiscount.h"
#include <map>
namespace minirisk {

PricerPayment::PricerPayment(const TradePayment& trd)
    : m_amt(trd.quantity())
    , m_dt(trd.delivery_date())
    , m_ir_curve(ir_curve_discount_name(trd.ccy()))
    , m_fx_ccy(trd.ccy() == "USD" ? "" : fx_spot_name(trd.ccy(),"USD"))
{
}

string my_date_transorm(string date_str)
{
    auto index_one = date_str.find('-', 0);
    auto index_two = date_str.find('-', index_one+1);
    
    return std::to_string(std::stoi(date_str.substr(0, index_one))) + '-' + std::to_string(std::stoi(date_str.substr(index_one+1,index_two))) + '-' + date_str.substr(index_two+1,date_str.size());

}
std::pair<double,string> PricerPayment::price(Market& mkt) const
{
    ptr_disc_curve_t disc = mkt.get_discount_curve(m_ir_curve);
    auto df_result = disc->df(m_dt); // this throws an exception if m_dt<today
    auto df = df_result.first;

    if(df>1)
    {
        if(df_result.second == 1) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Curve "+m_ir_curve+" DF not available before anchor date "+my_date_transorm(m_dt.to_string())+", requested "+my_date_transorm(Date(mkt.return_today().get_serial()+unsigned(df)).to_string()));
        else if(df_result.second == 2) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Curve "+m_ir_curve+" DF not available beyond last tenor date "+my_date_transorm(Date(mkt.return_today().get_serial()+unsigned(df)).to_string())+", requested "+my_date_transorm(m_dt.to_string()));
    }
    
    // This PV is expressed in m_ccy. It must be converted in USD.
    if (!m_fx_ccy.empty())
    {
        std::map<string, double> temp_map = mkt.get_fx_spot(m_fx_ccy);
        df *= ((temp_map.cbegin())->second);
    }
    // std::cout << m_amt * df << std::endl;
    return std::make_pair(m_amt * df, "");
}

} // namespace minirisk


