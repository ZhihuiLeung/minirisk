#include "MarketDataServer.h"
#include "Macros.h"
#include "Streamer.h"

#include <limits>
#include <vector>

namespace minirisk {

// transforms FX.SPOT.EUR.USD into FX.SPOT.EUR
string mds_spot_name(const string& name, const string& baseccy)
{
    // NOTE: in a real system error checks should be stricter, not just on the last 3 characters
    MYASSERT((name.substr(name.length() - 3, 3) == baseccy),
        "Only FX pairs in the format FX.SPOT.CCY." + baseccy + " can be queried. Got " << name);
    return name.substr(0, name.length() - 4);
}

MarketDataServer::MarketDataServer(const string& filename)
{
    std::ifstream is(filename);
    MYASSERT(!is.fail(), "Could not open file " << filename);
    do {
        string name;
        double value;
        is >> name >> value;
        // std::cout << name << " " << value << "\n";
        auto ins = m_data.emplace(name, value);
        MYASSERT(ins.second, "Duplicated risk factor: " << name);
    } while (is);
}

// get risk factors
std::map<string, double> MarketDataServer::get(const string& objtype, const string& name) const
{
    std::map<string, double> result_map;
    
    if(objtype == "yield curve")
    {
        auto pos = name.find('.', 0);

        auto matched_name_vec = match(name.substr(0, pos)+".(([1-9][0-9]*[DWY])|((1[0-2]|[1-9])[M]))."+name.substr(pos+1,name.size()));

        for(auto my_iter = matched_name_vec.cbegin(); my_iter != matched_name_vec.cend(); my_iter++)
        {
            auto iter = m_data.find(*my_iter);
            MYASSERT(iter != m_data.end(), "Market data not found: " << *my_iter);
            auto ins = result_map.emplace(*my_iter, iter->second);
            MYASSERT(ins.second, "Duplicated risk factor: " << *my_iter);
        }
    } 
    else if(objtype == "fx spot")
    {
        auto iter = m_data.find(name);
        MYASSERT(iter != m_data.end(), "Market data not found: " << name);
        auto ins = result_map.emplace(name, iter->second);
        MYASSERT(ins.second, "Duplicated risk factor: " << name);
    }    

    return result_map;
}

std::pair<double, bool> MarketDataServer::lookup(const string& name) const
{
    auto iter = m_data.find(name);
    return (iter != m_data.end())  // found?
            ? std::make_pair(iter->second, true)
            : std::make_pair(std::numeric_limits<double>::quiet_NaN(), false);
}

// return the name of risk factors whose name matched the RegExp
std::vector<std::string> MarketDataServer::match(const std::string& expr) const
{
    std::regex r(expr);
    std::vector<std::string> matched_name_vec;
    for(auto iter = m_data.cbegin(); iter != m_data.cend(); iter++)
    {
        if(std::regex_match(iter->first, r))
        {
            matched_name_vec.push_back(iter->first);
        }
    }

    return matched_name_vec;
}

} // namespace minirisk

