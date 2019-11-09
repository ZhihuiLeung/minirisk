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
        fds_data.emplace(name, std::map<Date, double>());
        auto ins = fds_data[name].emplace(Date(unsigned(std::atoi(date_str.c_str()))), value);
        MYASSERT(ins.second, "Duplicated risk factor: " << name << " " << date_str << " " << value);
    } while (is);
}

double FixingDataServer::get(const std::string& name, const Date& t) const {
    auto iter = fds_data.find(name);
    MYASSERT(iter != fds_data.end(), "Fixing data not found: " << name);

    auto date_iter = iter->second.find(t);
    MYASSERT(date_iter != iter->second.end(), "Fixing date not found: " << name << " " << t.to_string());

    return date_iter->second;
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

} // namespace minirisk