#include "Date.h"
#include "Macros.h"
#include <time.h>
#include <stdlib.h>

std::string my_transform(unsigned year, unsigned month, unsigned day)
{
    return std::to_string((year*100+month)*100+day);
}
bool my_calendar_format_validation(unsigned year, unsigned month, unsigned day)
{
    minirisk::Date my_date(year, month, day);

    std::string my_initial_date = my_transform(year, month, day);
    std::string result_date = my_date.to_string(false);

    if(my_initial_date != result_date) return false;

    return true;
}

bool my_serial_validation(int cur_serial, int lag_serial)
{
    return (cur_serial - lag_serial) == 1;
}
int get_cur_serial(unsigned year, unsigned month, unsigned day, int lag_serial)
{
    minirisk::Date my_date(year, month, day);
    int cur_serial = int(my_date.get_serial());

    MYASSERT(my_serial_validation(cur_serial, lag_serial), my_transform(year, month, day));
    return cur_serial;
}
void test1()
{
    // int my_seed = std::time(NULL);

    // std::cout << "Current tested seed: " << my_seed << std::endl;
    

    // std::cout << "test 1 passed!" << std::endl;
}

void test2()
{
    const std::array<unsigned, 12> days_in_month = { {31,28,31,30,31,30,31,31,30,31,30,31} };
    
    for(unsigned year = minirisk::Date::first_year; year < minirisk::Date::last_year; year++)
    {
        bool is_leap_year = minirisk::Date::is_leap_year(year);

        for(unsigned month = 1; month <= 12; month++)
        {
            for(unsigned day = 1; day <= days_in_month[month-1]; day++) MYASSERT(my_calendar_format_validation(year, month, day), my_transform(year, month, day));

            if(month == 2 && is_leap_year == true) MYASSERT(my_calendar_format_validation(year, month, 29), my_transform(year, month, 29));
        }
    }
    std::cout << "test 2 passed!" << std::endl;
}

void test3()
{
    const std::array<unsigned, 12> days_in_month = { {31,28,31,30,31,30,31,31,30,31,30,31} };

    bool is_leap_year;
    int lag_serial = -1;

    for(unsigned year = minirisk::Date::first_year; year < minirisk::Date::last_year; year++)
    {
        is_leap_year = minirisk::Date::is_leap_year(year);

        for(unsigned month = 1; month <= 12; month++)
        {
            for(unsigned day = 1; day <= days_in_month[month-1]; day++) lag_serial = get_cur_serial(year, month, day, lag_serial);

            if(month == 2 && is_leap_year == true) lag_serial = get_cur_serial(year, month, 29, lag_serial);
        }
    }
    std::cout << "test 3 passed!" << std::endl;
}

int main()
{
    test1();
    test2();
    test3();
    return 0;
}

