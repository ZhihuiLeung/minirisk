#include "CurveFXForward.h"
#include "Global.h"
#include "Market.h"

namespace minirisk {

CurveFXForward::CurveFXForward(
    Market *mkt, const Date& today, const std::string& name) 
    : m_today(today) {
      m_ccy = name.substr(7, 3);
      m_base_ccy = name.substr(11, 3);
      ccy_dc = mkt->get_discount_curve(ir_curve_discount_name(m_ccy));
      base_ccy_dc = mkt->get_discount_curve(ir_curve_discount_name(m_base_ccy));
      m_spot = mkt->get_fx_spot_curve(fx_spot_prefix + m_ccy);
    }

std::string CurveFXForward::name() const {
    return fx_forward_name(m_ccy, m_base_ccy);
}

double CurveFXForward::fwd(const Date& t) const {
  return m_spot->get_spot(fx_spot_prefix + m_base_ccy) * ccy_dc->df(t).first / base_ccy_dc->df(t).first;
}

} // namespace minirisk