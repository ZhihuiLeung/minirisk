#include "Market.h"
#include "CurveDiscount.h"

#include <vector>

namespace minirisk {

template <typename I, typename T>
std::shared_ptr<const I> Market::get_curve(const string& name)
{
    ptr_curve_t& curve_ptr = m_curves[name];
    if (!curve_ptr.get())
        curve_ptr.reset(new T(this, m_today, name));
    std::shared_ptr<const I> res = std::dynamic_pointer_cast<const I>(curve_ptr);
    MYASSERT(res, "Cannot cast object with name " << name << " to type " << typeid(I).name());
    return res;
}

const ptr_disc_curve_t Market::get_discount_curve(const string& name)
{
    return get_curve<ICurveDiscount, CurveDiscount>(name);
}

std::map<string, double> Market::from_mds(const string& objtype, const string& name)
{
    if(m_mds == NULL)
    {
        std::map<string, double> result_map;
        Market::vec_risk_factor_t temp;

        if(objtype == "yield curve")
        {
            auto pos = name.find('.', 0);
            temp = get_risk_factors(name.substr(0, pos)+".(([1-9][0-9]*[DWY])|((1[0-2]|[1-9])[M]))."+name.substr(pos+1,name.size()));

            for(auto iter=temp.cbegin(); iter!=temp.cend(); iter++)
            {
                result_map[iter->first] = iter->second;
            }
        } else if(objtype == "fx spot")
        {
            temp = get_risk_factors(name);
            result_map[(temp.cbegin())->first] = (temp.cbegin())->second;
        }
        return result_map;
    }
    
    std::map<string, double> result_map = m_mds->get(objtype, name);

    for(auto iter = result_map.cbegin(); iter != result_map.cend(); iter++)
    {
        auto ins = m_risk_factors.emplace(iter->first, std::numeric_limits<double>::quiet_NaN());
        if (ins.second) { // just inserted, need to be populated
            MYASSERT(m_mds, "Cannot fetch " << objtype << " " << iter->first << " because the market data server has been disconnnected");
            ins.first->second = iter->second;
        }
    }
    
    return result_map; 
}

const std::map<string, double> Market::get_yield(const string& ccyname)
{
    string name(ir_rate_prefix + ccyname);
    return from_mds("yield curve", name);
};

const std::map<string, double> Market::get_fx_spot(const string& name)
{
    return from_mds("fx spot", mds_spot_name(name));
}

void Market::set_risk_factors(const vec_risk_factor_t& risk_factors)
{
    clear();
    for (const auto& d : risk_factors) {
        auto i = m_risk_factors.find(d.first);
        MYASSERT((i != m_risk_factors.end()), "Risk factor not found " << d.first);
        i->second = d.second;
    }
}

Market::vec_risk_factor_t Market::get_risk_factors(const std::string& expr) const
{
    vec_risk_factor_t result;
    std::regex r(expr);
    for (const auto& d : m_risk_factors)
        if (std::regex_match(d.first, r))
            result.push_back(d);
    return result;
}

} // namespace minirisk
