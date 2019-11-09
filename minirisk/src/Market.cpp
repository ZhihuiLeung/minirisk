#include "Market.h"

#include "CurveDiscount.h"
#include "CurveFXSpot.h"
#include "CurveFXForward.h"

#include<map>
#include <vector>

namespace minirisk {

std::string get_fx_name(std::string name)
{
    auto pos = name.rfind('.');
    return name.substr(pos+1, name.size());
}

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

const ptr_fx_spot_curve_t Market::get_fx_spot_curve(const string& name) {
    return get_curve<ICurveFXSpot, CurveFXSpot>(name);
}

const ptr_fx_forward_curve_t Market::get_fx_fwd_curve(const string& name) {
    return get_curve<ICurveFXForward, CurveFXForward>(name);
}

std::map<string, double> Market::from_mds(const string& objtype, const string& name)
{
    std::map<string, double> result_map;
    if(m_mds == NULL)
    {
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
            temp = get_risk_factors("FX.SPOT.*");

            for(auto iter=temp.cbegin(); iter!=temp.cend(); iter++)
            {
                result_map.emplace(iter->first, iter->second);
            }
        }
    } else {
        result_map = m_mds->get(objtype, name);

        for(auto iter = result_map.cbegin(); iter != result_map.cend(); iter++)
        {
            auto ins = m_risk_factors.emplace(iter->first, std::numeric_limits<double>::quiet_NaN());
            if (ins.second) { // just inserted, need to be populated
                MYASSERT(m_mds, "Cannot fetch " << objtype << " " << iter->first << " because the market data server has been disconnnected");
                ins.first->second = iter->second;
            }
        }
    }
    return result_map; 
}

const std::map<string, double> Market::get_yield(const string& ccyname)
{
    string name(ir_rate_prefix + ccyname);
    return from_mds("yield curve", name);
};

const std::map<string, double> Market::generate_different_fx_spot(const string& name)
{
    std::map<string, double> result_map;

    if(m_mds == NULL) {
        auto iter = get_risk_factors(name);
        double cur_fx = iter.cbegin()->second;

        auto risk_factor_vec = get_risk_factors("FX.SPOT.*");

        for(auto cur_risk_factor_iter=risk_factor_vec.cbegin(); cur_risk_factor_iter!=risk_factor_vec.cend(); cur_risk_factor_iter++)
        {
            if(cur_risk_factor_iter->first != name)
            {
                result_map.emplace(cur_risk_factor_iter->first, cur_fx / cur_risk_factor_iter->second);
            } else {
                result_map.emplace(cur_risk_factor_iter->first.substr(0, cur_risk_factor_iter->first.size()-3)+"USD", cur_fx);
            }
        }
    } else {

        auto temp = m_mds->get("fx spot", name);
        double cur_fx = temp[name];

        auto ins = m_risk_factors.emplace(name, std::numeric_limits<double>::quiet_NaN());
        if (ins.second) { // just inserted, need to be populated
            MYASSERT(m_mds, "Cannot fetch " << "fx spot" << " " << name << " because the market data server has been disconnnected");
            ins.first->second = cur_fx;
        }
        
        auto all_fx_name_vec = m_mds->match("FX.SPOT.*");

        for(auto cur_fx_iter=all_fx_name_vec.cbegin(); cur_fx_iter!=all_fx_name_vec.cend(); cur_fx_iter++)
        {
            temp = m_mds->get("fx spot", *cur_fx_iter);
            if((*cur_fx_iter) != name)
            {
                result_map.emplace((*cur_fx_iter), cur_fx / temp[(*cur_fx_iter)]);
            } else {
                result_map.emplace((*cur_fx_iter).substr(0, (*cur_fx_iter).size()-3)+"USD", cur_fx);
            }
        }
    }
    return result_map;
}
const std::map<string, double> Market::get_fx_spot(const string& name)
{
    return generate_different_fx_spot(name);
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
const std::string Market::get_baseccy()
{
    return m_baseccy;
}
} // namespace minirisk
