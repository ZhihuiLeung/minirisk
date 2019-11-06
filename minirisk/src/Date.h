#pragma once

#include "Macros.h"
#include <string>
#include <array>

namespace minirisk {

struct Date
{
public:
    static const unsigned first_year = 1900;
    static const unsigned last_year = 2200;
    static const unsigned n_years = last_year - first_year;

private:
    static std::string padding_dates(unsigned);

    // number of days elapsed from beginning of the year
    unsigned day_of_year(unsigned month, unsigned day, bool lomo_is_leap) const;

    unsigned get_initial_format_date() const;

    friend long operator-(const Date& d1, const Date& d2);

    static const std::array<unsigned, 12> days_in_month;  // num of days in month M in a normal year
    static const std::array<unsigned, 12> days_ytd;      // num of days since 1-jan to 1-M in a normal year
    static const std::array<unsigned, 12> leap_days_ytd;
    static const std::array<unsigned, n_years> days_epoch;   // num of days since 1-jan-1900 to 1-jan-yyyy (until 2200)

public:
    // Default constructor
    // Date() : m_y(1970), m_m(1), m_d(1), m_is_leap(false) {}
    Date() : lomo_serial(0) {}

    // Constructor where the input value is checked.
    Date(unsigned year, unsigned month, unsigned day)
    {
        init(year, month, day);
    }

    Date(unsigned serial) : lomo_serial(serial) {}

    void set_lomo_serial(unsigned serial)
    {
        lomo_serial = serial;
    }

    void init(unsigned year, unsigned month, unsigned day)
    {
        check_valid(year, month, day);

        bool lomo_is_leap = is_leap_year(year);

        lomo_serial = this->serial(year, month, day, lomo_is_leap);
    }

    static void check_valid(unsigned y, unsigned m, unsigned d);

    bool operator<(const Date& d) const
    {
        return (lomo_serial < d.get_serial());
    }

    bool operator==(const Date& d) const
    {
        return (lomo_serial == d.get_serial());
    }

    bool operator>(const Date& d) const
    {
        return (lomo_serial > d.get_serial());
    }

    unsigned get_serial() const
    {
        return lomo_serial;
    }

    // number of days since 1-Jan-1900
    unsigned serial(unsigned year, unsigned month, unsigned day, bool lomo_is_leap) const
    {
        return days_epoch[year - 1900] + day_of_year(month, day, lomo_is_leap);
    }

    static bool is_leap_year(unsigned yr);

    // In YYYYMMDD format
    std::string to_string(bool pretty = true) const
    {
        std::string my_date = std::to_string(get_initial_format_date());

        std::string m_y = my_date.substr(0, 4);
        std::string m_m = my_date.substr(4, 2);
        std::string m_d = my_date.substr(6, 2);
        unsigned m = static_cast<unsigned>(std::atoi(m_m.c_str()));
        unsigned d = static_cast<unsigned>(std::atoi(m_d.c_str()));
        return pretty
            ? m_d + "-" + m_m + "-" + m_y
            : m_y + padding_dates(m) + padding_dates(d);
    }

private:
    unsigned lomo_serial;
};

long operator-(const Date& d1, const Date& d2);

inline double time_frac(const Date& d1, const Date& d2)
{
    return static_cast<double>(d2 - d1) / 365.0;
}

} // namespace minirisk
