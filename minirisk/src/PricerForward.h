#include "IPricer.h"
#include "TradeFXForward.h"

namespace minirisk {

struct PricerForward : IPricer {
public:
    PricerForward(const TradeFXForward& trd, const std::string& base_ccy);
    virtual std::pair<double, string> price(Market& m, const FixingDataServer& fds) const;
private:
    double m_amt;
    double m_strike;
    std::string m_fwd_base_ccy;
    std::string m_fwd_quote_ccy;
    Date m_fwd_fixing_date;
    Date m_fwd_settlement_date;
    std::string m_base_ccy;
};

} // namespace minirisk