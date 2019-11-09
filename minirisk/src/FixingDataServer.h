#include <string>
#include <map>

#include "Date.h"

namespace minirisk {

struct FixingDataServer {
public:
    FixingDataServer(const std::string& filename);
    FixingDataServer();
    double get(const std::string& name, const Date& t) const;
    std::pair<double, bool> lookup(const std::string& name, const Date& t) const;

private:
    std::map<std::string, std::map<Date, double>> fds_data;
};

} // namespace minirisk