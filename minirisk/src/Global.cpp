#include "Global.h"
#include <iomanip>
#include <sstream>

namespace minirisk {

const string ir_rate_prefix = "IR.";
const string ir_curve_discount_prefix = "IR.DISCOUNT.";
const string fx_spot_prefix = "FX.SPOT.";
const string fx_forward_prefix = "FX.FWD.";

string format_label(const string& s)
{
    std::ostringstream os;
    os << std::setw(20) << std::left << s;
    return os.str();
}

string my_date_transform(string date_str)
{
    auto index_one = date_str.find('-', 0);
    auto index_two = date_str.find('-', index_one+1);
    
    return std::to_string(std::stoi(date_str.substr(0, index_one))) + '-' + std::to_string(std::stoi(date_str.substr(index_one+1,index_two))) + '-' + date_str.substr(index_two+1,date_str.size());
}

}
