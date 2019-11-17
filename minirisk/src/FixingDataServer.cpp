#include "FixingDataServer.h"

#include <fstream>

#include "Macros.h"
#include "Global.h"

namespace minirisk {

FixingDataServer::FixingDataServer(const std::string& filename) {
    std::ifstream is(filename);
    MYASSERT(!is.fail(), "Could not open file " << filename);
    do {
        string name;
        std::string date_str;
        double value;
        is >> name >> date_str >> value;
        // std::cout << name << " " << value << "\n";
        if(name == "") break;

        // read the date format of YYYYMMDD from fixing.txt
        unsigned year = unsigned(std::atoi(date_str.substr(0,4).c_str()));
        unsigned month = unsigned(std::atoi(date_str.substr(4,2).c_str()));
        unsigned day = unsigned(std::atoi(date_str.substr(6,2).c_str()));

        fds_data.emplace(name, std::map<Date, double>());
        auto ins = fds_data[name].emplace(Date(year, month, day), value);
        MYASSERT(ins.second, "Duplicated risk factor: " << name << " " << date_str << " " << value);
    } while (is);
}

std::pair<double, std::string> FixingDataServer::get(const std::string& name, const Date& t) const {
    
    auto iter = fds_data.find(name);

    // name not found
    if(iter == fds_data.end()) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Fixing not found: " + name + "," + t.to_string());
    auto date_iter = iter->second.find(t);

    // date not found
    if(date_iter == iter->second.end()) return std::make_pair(std::numeric_limits<double>::quiet_NaN(), "Fixing not found: " + name + "," + t.to_string());
    
    return std::make_pair(date_iter->second, "");
}

std::pair<double, bool> FixingDataServer::lookup(const std::string& name, const Date& t) const {
    auto iter = fds_data.find(name);
    if (iter != fds_data.end())
    {
        auto date_iter = iter->second.find(t);

        if (date_iter != iter->second.end()) return std::make_pair(date_iter->second, true);
    }
  return std::make_pair(std::numeric_limits<double>::quiet_NaN(), false);
}

bool FixingDataServer::is_empty() const
{
    return(fds_data.size() == 0);
}
} // namespace minirisk