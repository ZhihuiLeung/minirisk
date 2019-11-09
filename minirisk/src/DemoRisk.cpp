#include <iostream>
#include <algorithm>

#include "MarketDataServer.h"
#include "PortfolioUtils.h"

using namespace::minirisk;

void run(const string& portfolio_file, const string& risk_factors_file, const string& baseccy, const string& fixing_file_path)
{
    // load the portfolio from file
    portfolio_t portfolio = load_portfolio(portfolio_file);
    // save and reload portfolio to implicitly test round trip serialization
    save_portfolio("portfolio.tmp", portfolio);
    portfolio.clear();
    portfolio = load_portfolio("portfolio.tmp");

    // display portfolio
    print_portfolio(portfolio);

    // get pricers
    std::vector<ppricer_t> pricers(get_pricers(portfolio, baseccy));

    // initialize market data server
    std::shared_ptr<const MarketDataServer> mds(new MarketDataServer(risk_factors_file));
    FixingDataServer fds(fixing_file_path);
    
    // Init market object
    Date today(2017,8,5);
    Market mkt(mds, today, baseccy, fds);

    // Price all products. Market objects are automatically constructed on demand,
    // fetching data as needed from the market data server.
    {

        auto prices = compute_prices(pricers, mkt);
        print_price_vector("PV", prices);
    }

    // disconnect the market (no more fetching from the market data server allowed)
    mkt.disconnect();

    // display all relevant risk factors
    {
        std::cout << "Risk factors:\n";
        auto tmp = mkt.get_risk_factors(".+");
        for (const auto& iter : tmp)
            std::cout << iter.first << "\n";
        std::cout << "\n";
    }

    {   // Compute PV01 (i.e. sensitivity with respect to interest rate dV/dr)
        std::vector<std::pair<string, portfolio_values_t>> pv01(compute_pv01(pricers,mkt));  // PV01 per trade
        std::vector<std::pair<string, portfolio_values_t>> fx_delta(compute_fx_delta(pricers,mkt));  // PV01 per trade
        // display PV01 per currency
        for (const auto& g : pv01)
            print_price_vector("PV01 " + g.first, g.second);
        for (const auto& g : fx_delta)
            print_price_vector("FX delta " + g.first, g.second);
    }
}

void usage()
{
    std::cerr
        << "Invalid command line arguments\n"
        << "Example:\n"
        << "DemoRisk -p portfolio.txt -f risk_factors.txt\n";
    std::exit(-1);
}

int main(int argc, const char **argv)
{
    // parse command line arguments
    string portfolio, riskfactors;
    string baseccy = "USD";
    string fixing_file_path = "";
    if (argc % 2 == 0)
        usage();
    for (int i = 1; i < argc; i += 2) {
        string key(argv[i]);
        string value(argv[i+1]);
        if (key == "-p")
            portfolio = value;
        else if (key == "-f")
            riskfactors = value;
        else if (key == "-b")
			baseccy = value;
        else if (key == "-x")
            fixing_file_path = value;
        else
            usage();
    }
    if (portfolio == "" || riskfactors == "")
        usage();

    try {
        fixing_file_path = "./data/" + fixing_file_path;
        run(portfolio, riskfactors, baseccy, fixing_file_path);
        return 0;  // report success to the caller
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return -1; // report an error to the caller
    }
    catch (...)
    {
        std::cerr << "Unknown exception occurred\n";
        return -1; // report an error to the caller
    }
}
