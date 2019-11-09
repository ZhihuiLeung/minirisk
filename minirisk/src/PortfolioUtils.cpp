#include "Global.h"
#include "PortfolioUtils.h"
#include "TradePayment.h"

#include <numeric>
#include <set>
namespace minirisk {

void print_portfolio(const portfolio_t& portfolio)
{
    std::for_each(portfolio.begin(), portfolio.end(), [](auto& pt){ pt->print(std::cout); });
}

std::vector<ppricer_t> get_pricers(const portfolio_t& portfolio, const std::string& baseccy)
{
    std::vector<ppricer_t> pricers(portfolio.size());
    std::transform( portfolio.begin(), portfolio.end(), pricers.begin()
                  , [baseccy](auto &pt) -> ppricer_t { return pt->pricer(baseccy); } );
    return pricers;
}

portfolio_values_t compute_prices(const std::vector<ppricer_t>& pricers, Market& mkt)
{
    portfolio_values_t prices(pricers.size());
    std::transform(pricers.begin(), pricers.end(), prices.begin()
        ,[&mkt](auto &pp) -> std::pair<double,string> { return pp->price(mkt);});
    return prices;
}

std::pair<double, std::vector<std::pair<size_t,string>>> portfolio_total(const portfolio_values_t& values)
{
    std::vector<double> valid_vec;
    std::vector<std::pair<size_t,string>> invalid_vec;
    for(auto iter = values.cbegin(); iter != values.cend(); iter++)
    {
        if((iter->second == ""))
        {
            valid_vec.push_back(iter->first);
        } else {
            invalid_vec.push_back(std::make_pair(size_t(iter-values.cbegin()), iter->second));
        }
    }
    return std::make_pair(std::accumulate(valid_vec.begin(), valid_vec.end(), 0.0), invalid_vec);
}

std::vector<std::pair<string, portfolio_values_t>> compute_parallel_pv01(const std::vector<ppricer_t>& pricers, const Market& mkt)
{
    std::vector<std::pair<string, portfolio_values_t>> pv01;  // PV01 per trade
    std::set<string> my_set;

    const double bump_size = 0.01 / 100;
    // bucketed 
    // filter risk factors related to IR
    auto base = mkt.get_risk_factors(ir_rate_prefix + "(([1-9][0-9]*[DWY])|((1[0-2]|[1-9])[M])).*");

    // compute prices for perturbated markets and aggregate results
    pv01.reserve(base.size());

    for (const auto& d : base) {
        string temp = d.first;
        my_set.insert(temp.substr(temp.size()-3, temp.size()));
    }
    for(auto iter=my_set.begin(); iter!=my_set.end();iter++)
    {
        portfolio_values_t pv_up, pv_dn;
        double dr = 2.0 * bump_size;

        // Make a local copy of the Market object, because we will modify it applying bumps
        // Note that the actual market objects are shared, as they are referred to via pointers
        Market tmpmkt_up(mkt);
        Market tmpmkt_down(mkt);

        base = mkt.get_risk_factors(ir_rate_prefix + "(([1-9][0-9]*[DWY])|((1[0-2]|[1-9])[M]))."+(*iter));
        
        pv01.push_back(std::make_pair("parallel " + ir_rate_prefix+(*iter), portfolio_values_t(pricers.size())));

        for (const auto& d : base) {
            std::vector<std::pair<string, double>> bumped(1, d);
            // std::cout << d.first << std::endl;
            bumped[0].second = d.second - bump_size;
            tmpmkt_down.set_risk_factors(bumped);
            
            bumped[0].second = d.second + bump_size; // bump up
            tmpmkt_up.set_risk_factors(bumped);
        }

        pv_dn = compute_prices(pricers, tmpmkt_down);
        pv_up = compute_prices(pricers, tmpmkt_up);

        std::transform(pv_up.begin(), pv_up.end(), pv_dn.begin(), pv01.back().second.begin()
            , [dr](std::pair<double, string> hi, std::pair<double, string> lo) -> std::pair<double, string> {
                if(hi.second == "" && lo.second == "")
                {
                    return std::make_pair((hi.first - lo.first) / dr, "");
                } else {
                    return std::make_pair(std::numeric_limits<double>::quiet_NaN(), hi.second);
                }
                });
        // std::transform(pv_up.begin(), pv_up.end(), pv_dn.begin(), pv01.back().second.begin()
        //         , [dr](double hi, double lo) -> double { return (hi - lo) / dr; });
    }
    
    return pv01;
}

std::vector<std::pair<string, portfolio_values_t>> compute_bucketed_pv01(const std::vector<ppricer_t>& pricers, const Market& mkt)
{
    std::vector<std::pair<string, portfolio_values_t>> pv01;  // PV01 per trade

    const double bump_size = 0.01 / 100;
    // bucketed 
    // filter risk factors related to IR
    auto base = mkt.get_risk_factors(ir_rate_prefix + "(([1-9][0-9]*[DWY])|((1[0-2]|[1-9])[M])).*");

    // Make a local copy of the Market object, because we will modify it applying bumps
    // Note that the actual market objects are shared, as they are referred to via pointers
    Market tmpmkt(mkt);

    // compute prices for perturbated markets and aggregate results
    pv01.reserve(base.size());

    for (const auto& d : base) {
        portfolio_values_t pv_up, pv_dn;
        std::vector<std::pair<string, double>> bumped(1, d);
        pv01.push_back(std::make_pair("bucketed " + d.first, portfolio_values_t(pricers.size())));

        // bump down and price
        bumped[0].second = d.second - bump_size;
        tmpmkt.set_risk_factors(bumped);
        pv_dn = compute_prices(pricers, tmpmkt);

        // bump up and price
        bumped[0].second = d.second + bump_size; // bump up
        tmpmkt.set_risk_factors(bumped);
        pv_up = compute_prices(pricers, tmpmkt);

        // restore original market state for next iteration
        // (more efficient than creating a new copy of the market at every iteration)
        bumped[0].second = d.second;
        tmpmkt.set_risk_factors(bumped);

        // compute estimator of the derivative via central finite differences
        double dr = 2.0 * bump_size;
        std::transform(pv_up.begin(), pv_up.end(), pv_dn.begin(), pv01.back().second.begin()
            , [dr](std::pair<double, string> hi, std::pair<double, string> lo) -> std::pair<double, string> {
                if(hi.second == "" && lo.second == "")
                {
                    return std::make_pair((hi.first - lo.first) / dr, "");
                } else {
                    return std::make_pair(std::numeric_limits<double>::quiet_NaN(), hi.second);
                }
                });
    }
    return pv01;
}

std::vector<std::pair<string, portfolio_values_t>> compute_pv01(const std::vector<ppricer_t>& pricers, const Market& mkt)
{
    auto bucketed_pv01 = compute_bucketed_pv01(pricers, mkt);
    auto parallel_pv01 = compute_parallel_pv01(pricers, mkt);

    std::vector<std::pair<string, portfolio_values_t>> pv01_result(bucketed_pv01.size()+parallel_pv01.size());
    std::merge(bucketed_pv01.begin(), bucketed_pv01.end(), parallel_pv01.begin(), parallel_pv01.end(), pv01_result.begin());
    return pv01_result;
}

// std::vector<std::pair<std::string, portfolio_values_t>> compute_fx_delta(
//      const std::vector<ppricer_t>& pricers, const Market& mkt,
//      std::shared_ptr<const FixingDataServer> fds) {
//   std::vector<std::pair<std::string, portfolio_values_t>> fx_delta;
//   auto fx_spots = mkt.get_risk_factors(fx_spot_prefix + "[A-Z]{3}");
//   std::vector<std::string> risk_ccys;
//   find_all_risk_ccy(fx_spots, &risk_ccys);
//   Market tmpmkt(mkt);

//   for (const auto& risk_ccy : risk_ccys) {
//     auto risk_factors = mkt.get_risk_factors(fx_spot_prefix + risk_ccy);
//     MYASSERT(risk_factors.size() == 1, 
//         "Duplicate fx spot rate." << fx_spot_prefix + risk_ccy);
//     const double original_value = risk_factors[0].second;
//     double bump_size = original_value * 0.1 / 100;
//     double dr = 2 * bump_size;
//     risk_factors[0].second = original_value + bump_size;
//     tmpmkt.set_risk_factors(risk_factors);
//     auto pv_up = compute_prices(pricers, tmpmkt, fds);
//     risk_factors[0].second = original_value - bump_size;
//     tmpmkt.set_risk_factors(risk_factors);
//     auto pv_dn = compute_prices(pricers, tmpmkt, fds);
//     risk_factors[0].second = original_value;
//     tmpmkt.set_risk_factors(risk_factors);

//     fx_delta.push_back(
//         std::make_pair(fx_spot_prefix + risk_ccy,
//           std::vector<trade_value_t>(pricers.size())));

//     std::transform(
//         pv_up.begin(), pv_up.end(), pv_dn.begin(), 
//         fx_delta.back().second.begin(), [dr](auto& hi, auto& lo) ->
//         trade_value_t { return pv01_or_nan(hi, lo, dr); });
//   }
//   return fx_delta;
// }
ptrade_t load_trade(my_ifstream& is)
{
    string name;
    ptrade_t p;

    // read trade identifier
    guid_t id;
    is >> id;

    if (id == TradePayment::m_id)
        p.reset(new TradePayment);
    else
        THROW("Unknown trade type:" << id);

    p->load(is);

    return p;
}

void save_portfolio(const string& filename, const std::vector<ptrade_t>& portfolio)
{
    // test saving to file
    my_ofstream of(filename);
    for( const auto& pt : portfolio) {
        pt->save(of);
        of.endl();
    }
    of.close();
}

std::vector<ptrade_t> load_portfolio(const string& filename)
{
    std::vector<ptrade_t> portfolio;

    // test reloading the portfolio
    my_ifstream is(filename);
    while (is.read_line())
        portfolio.push_back(load_trade(is));

    return portfolio;
}

void print_price_vector(const string& name, const portfolio_values_t& values)
{
    auto temp = portfolio_total(values);
    std::cout
        << "========================\n"
        << name << ":\n"
        << "========================\n"
        << "Total: " << temp.first << std::endl
        << "Errors: " << temp.second.size() << std::endl
        << "\n========================\n";

    for (size_t i = 0, n = values.size(); i < n; ++i)
    if(values[i].second == "")
    {
        std::cout << std::setw(5) << i << ": " << values[i].first << "\n";
    } else {
        std::cout << std::setw(5) << i << ": " << values[i].second << "\n";
    }

    std::cout << "========================\n\n";
}

} // namespace minirisk
