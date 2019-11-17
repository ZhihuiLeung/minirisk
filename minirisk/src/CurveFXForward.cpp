#include "CurveFXForward.h"
#include "Global.h"
#include "Market.h"

namespace minirisk {

CurveFXForward::CurveFXForward(Market *mkt, const Date& today, const std::string& name) 
{
    m_today = today;
    // ex. name = FX.SPOT.EUR.USD;
    // EUR
    m_ccy = name.substr(7, 3);
    // USD
    m_base_ccy = name.substr(11, 3);
    ccy_dc = mkt->get_discount_curve(ir_curve_discount_name(m_ccy));
    base_ccy_dc = mkt->get_discount_curve(ir_curve_discount_name(m_base_ccy));
    
    if(m_ccy != "USD")
    {
        m_spot = mkt->get_fx_spot_curve(fx_spot_prefix + m_ccy);
    } else {
        m_spot = mkt->get_fx_spot_curve(fx_spot_prefix + m_base_ccy);
    }
}

std::string CurveFXForward::name() const {
    return fx_forward_name(m_ccy, m_base_ccy);
}

double CurveFXForward::fwd(const Date& t) const {
    if(m_ccy != "USD")
    {
        return m_spot->get_spot(fx_spot_prefix + m_base_ccy) * ccy_dc->df(t).first / base_ccy_dc->df(t).first;
    } else {
        auto temp_1 = 1 / m_spot->get_spot(fx_spot_prefix + m_ccy);
        auto temp_2 = ccy_dc->df(t).first;
        auto temp_3 = base_ccy_dc->df(t).first;
        return temp_1 * temp_2 * temp_3;
    }
}

} // namespace minirisk