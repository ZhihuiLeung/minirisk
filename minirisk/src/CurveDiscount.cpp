#include "CurveDiscount.h"
#include "Market.h"
#include "Streamer.h"

#include <cmath>


namespace minirisk {

double my_tenor_transform(std::string tenor_str)
{
    auto i_one = tenor_str.find('.', 0);
    auto i_two = tenor_str.find('.', i_one+1);
    double tenor_num_part = static_cast<double>(std::atoi(tenor_str.substr(i_one+1, i_two-1).c_str()));
    char tenor_char_part = tenor_str[i_two-1];

    // auto temp = CurveDiscount::tenor_char_part_map[tenor_char_part];

    double temp;
    switch (tenor_char_part)
    {
    case 'D':
        temp = 1;
        break;
    case 'W':
        temp = 7;
        break;
    case 'M':
        temp = 30;
        break;
    case 'Y':
        temp = 365;
        break;
    default:
        temp = 0;
        std::cout << "tenor_char_part error" << std::endl;
    }
    return tenor_num_part * temp / 365;
    
}

bool judge(const std::pair<int,double> a, const std::pair<int ,double> b) {
    return a.first<b.first;
}

CurveDiscount::CurveDiscount(Market *mkt, const Date& today, const string& curve_name)
    : m_today(today)
    , m_name(curve_name)
    , name_rate_map(mkt->get_yield(curve_name.substr(ir_curve_discount_prefix.length(),3)))
{
    
    double cur_tenor = 0;

    for(auto iter = name_rate_map.cbegin(); iter != name_rate_map.cend(); iter++)
    {
        cur_tenor = my_tenor_transform(iter->first);
        tenor_rate_vec.push_back(std::make_pair(cur_tenor, iter->second));
    }
    sort(tenor_rate_vec.begin(),tenor_rate_vec.end(), judge);
    
    for(auto iter = tenor_rate_vec.cbegin(); iter != tenor_rate_vec.cend(); iter++)
    {
        rate_mul_tenor_vec.push_back(- (iter->first) * (iter->second));
    }
    
}

std::pair<double, unsigned> CurveDiscount::df(const Date& t) const
{
    if(t < m_today) return std::make_pair(t.get_serial()-m_today.get_serial(), 1);
    
    double dt = time_frac(m_today, t);

    unsigned id = unsigned(std::lower_bound(tenor_rate_vec.begin(), tenor_rate_vec.end(), std::make_pair(dt, 0), judge) - tenor_rate_vec.begin());

    if(id == tenor_rate_vec.size()) return std::make_pair(tenor_rate_vec.back().first * 365, 2);

    double t_i = tenor_rate_vec[id].first;
    double t_i_add_one = tenor_rate_vec[id+1].first;
    
    double remain_tenor = dt - t_i;

    if(remain_tenor == 0)
    {
        return std::make_pair(std::exp(rate_mul_tenor_vec[id]), 0);
    } else {
        double forward_rate = (-rate_mul_tenor_vec[id+1] + rate_mul_tenor_vec[id]) / (t_i_add_one - t_i);
        double forward_rate_mul_tenor_diff = - forward_rate * remain_tenor;
        return std::make_pair(std::exp(rate_mul_tenor_vec[id] + forward_rate_mul_tenor_diff), 0);
    }
}

} // namespace minirisk
