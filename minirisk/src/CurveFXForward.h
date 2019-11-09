#pragma once

#include <string>

#include "ICurve.h"

namespace minirisk {

struct Market;

struct CurveFXForward : ICurveFXForward {
public:
    virtual std::string name() const;

    virtual Date today() const { return m_today; }

    CurveFXForward(Market *mkt, const Date& today, const std::string& name);

    virtual double fwd(const Date& t) const;

private:
    Date m_today;
    ptr_disc_curve_t ccy_dc;
    ptr_disc_curve_t base_ccy_dc;
    ptr_fx_spot_curve_t m_spot;
    std::string m_ccy;
    std::string m_base_ccy;
}; // struct CurveFXForward

} // namespace minirisk