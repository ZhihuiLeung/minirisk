#pragma once
#include "ICurve.h"
#include <map>
#include <vector>

namespace minirisk {

struct Market;

struct CurveDiscount : ICurveDiscount
{
    static std::map<char, double> tenor_char_part_map;

    virtual string name() const { return m_name; }

    CurveDiscount(Market *mkt, const Date& today, const string& curve_name);

    // compute the discount factor
    std::pair<double, unsigned> df(const Date& t) const;

    virtual Date today() const { return m_today; }

private:
    Date   m_today;
    string m_name;
    std::map<string, double> name_rate_map;
    std::vector<std::pair<double, double>> tenor_rate_vec;
    std::vector<double> rate_mul_tenor_vec;
};

} // namespace minirisk
