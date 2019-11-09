#include <string>
#include "ICurve.h"

namespace minirisk {

struct Market;

struct CurveFXSpot : ICurveFXSpot {
public:
    virtual std::string name() const { return m_name; }

    virtual Date today() const { return m_today; }

    CurveFXSpot(Market *mkt, const Date& today, const std::string& name);

    double get_spot(const std::string& name) const {
        return (fx_map.find(name)->second);
        }


private:
    Date m_today;
    std::string m_name;
    std::map<string, double> fx_map;
}; // struct CurveFXSpot

} // namespace minirisk