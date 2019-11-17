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

    // get today
    virtual Date today() const { return m_today; }

private:
    Date   m_today;
    string m_name;
    // a map with key = currency_tenor and value = interest
    std::map<string, double> name_rate_map;
    // a vector pair tenor and interest
    std::vector<std::pair<double, double>> tenor_rate_vec;
    // a temp_vec to store the r_i * t_i
    std::vector<double> rate_mul_tenor_vec;
};

} // namespace minirisk
