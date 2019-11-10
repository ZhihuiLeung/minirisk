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

double my_hex_to_dec(string hex_str)
{
    union { double d; uint64_t u; } tmp;

    std::stringstream ss;

    ss << std::hex << hex_str;
    ss >> tmp.u;

    return tmp.d;
}

string my_dec_to_hex(double dec)
{
    union { double d; uint64_t u; } tmp;
    std::stringstream ss;
    std::string hex_str;
    
    tmp.d = dec;
    ss << std::hex << tmp.u;
    ss >> hex_str;

    return hex_str;
}
}
