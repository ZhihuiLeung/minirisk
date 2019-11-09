#include "TradePayment.h"
#include "PricerPayment.h"

namespace minirisk {

ppricer_t TradePayment::pricer(const std::string& baseccy) const
{
    return ppricer_t(new PricerPayment(*this, baseccy));
}

} // namespace minirisk
