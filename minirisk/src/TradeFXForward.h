#include "Trade.h"
#include "Date.h"

namespace minirisk {

struct TradeFXForward : Trade<TradeFXForward> {
    friend struct Trade<TradeFXForward>;

    static const guid_t m_id;
    static const std::string m_name;

    TradeFXForward() {}

    void init(const std::string& base_ccy, const std::string& quote_ccy, const double quantity, const double strike, const Date& fixing_date, const Date& settlement_date) {
        Trade::init(quantity);
        m_base_ccy = base_ccy;
        m_quote_ccy = quote_ccy;
        m_strike = strike;
        m_fixing_date = fixing_date;
        m_settlement_date = settlement_date;
    }

    virtual ppricer_t pricer(const std::string& quote_ccy) const;

    const string& get_base_ccy() const { return m_base_ccy; }
    const string& get_quote_ccy() const { return m_quote_ccy; }
    double get_strike() const { return m_strike; }
    const Date& get_fixing_date() const { return m_fixing_date; }
    const Date& get_settlement_date() const { return m_settlement_date; }

private:
    void save_details(my_ofstream& os) const {
        os << m_base_ccy << m_quote_ccy << my_dec_to_hex(m_strike) << m_fixing_date << m_settlement_date;
    }

    void load_details(my_ifstream& is) {
        std::string m_strike_temp;
        is >> m_base_ccy >> m_quote_ccy >> m_strike_temp >> m_fixing_date >> m_settlement_date;
        
        m_strike = my_hex_to_dec(m_strike_temp);
    }

    void print_details(std::ostream& os) const {
        os << format_label("Strike level") << m_strike << std::endl;
        os << format_label("Base Currency") << m_base_ccy << std::endl;
        os << format_label("Quote Currency") << m_quote_ccy << std::endl;
        os << format_label("Fixing Date") << m_fixing_date.to_string() << std::endl;
        os << format_label("Settlement Date") << m_settlement_date.to_string() << std::endl;
    }

    std::string m_base_ccy;
    std::string m_quote_ccy;

    double m_strike;
    
    Date m_fixing_date;
    Date m_settlement_date;
};

} // namespace minirisk