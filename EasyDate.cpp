//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================

#include <time.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>

#include "EasyDate.hpp"

EasyDate::EasyDate()
:   m_year(1970), m_mon(1), m_yday(0),
    m_mday(1), m_wday(0), m_hour(0),
    m_min(0), m_sec(0), m_offUtc(36000)
{
    tzset();
    setCurrentTime();
}

EasyDate::~EasyDate()
{
}

EasyDate::EasyDate(const EasyDate& copy)
:   m_year(copy.m_year), m_mon(copy.m_mon), m_yday(copy.m_yday),
    m_mday(copy.m_mday), m_wday(copy.m_wday), m_hour(copy.m_hour),
    m_min(copy.m_min), m_sec(copy.m_sec), m_offUtc(copy.m_offUtc)
{
}

EasyDate& EasyDate::operator=(const EasyDate& rhs)
{
    if (this != &rhs)
    {
        m_year   = rhs.m_year;
        m_mon    = rhs.m_mon;
        m_yday   = rhs.m_yday;
        m_mday   = rhs.m_mday;
        m_wday   = rhs.m_wday;
        m_hour   = rhs.m_hour;
        m_min    = rhs.m_min;
        m_sec    = rhs.m_sec;
        m_offUtc = rhs.m_offUtc;
    }
    return *this;
}

// Comparison operators.
bool EasyDate::operator==(const EasyDate& rhs) const
{
    // We don't compare the UTC offset as the time attributes
    // are already normalised to UTC.
    return (m_year == rhs.m_year &&
            m_mon  == rhs.m_mon &&
            m_mday == rhs.m_mday &&
            m_hour == rhs.m_hour &&
            m_min  == rhs.m_min &&
            m_sec  == rhs.m_sec);
}

bool EasyDate::operator<(const EasyDate& rhs) const
{
    if (m_year < rhs.m_year)
        return true;
    if (m_year == rhs.m_year)
    {
        if (m_mon < rhs.m_mon)
            return true;
        if (m_mon == rhs.m_mon)
        {
            if (m_mday < rhs.m_mday)
                return true;
            if (m_mday == rhs.m_mday)
            {
                if (m_hour < rhs.m_hour)
                    return true;
                if (m_hour == rhs.m_hour)
                {
                    if (m_min < rhs.m_min)
                        return true;
                    if (m_min == rhs.m_min)
                    {
                        if (m_sec < rhs.m_sec)
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

bool EasyDate::operator>(const EasyDate& rhs) const
{
    if (m_year > rhs.m_year)
        return true;
    if (m_year == rhs.m_year)
    {
        if (m_mon > rhs.m_mon)
            return true;
        if (m_mon == rhs.m_mon)
        {
            if (m_mday > rhs.m_mday)
                return true;
            if (m_mday == rhs.m_mday)
            {
                if (m_hour > rhs.m_hour)
                    return true;
                if (m_hour == rhs.m_hour)
                {
                    if (m_min > rhs.m_min)
                        return true;
                    if (m_min == rhs.m_min)
                    {
                        if (m_sec > rhs.m_sec)
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

// Attribute getters.
std::string EasyDate::getMonName() const
{
    static const char* monName[] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    return monName[m_mon - 1];
}

std::string EasyDate::getWDayName() const
{
    static const char* dayShortName[] =
    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    return dayShortName[m_wday];
}

std::string EasyDate::getWDayLongName() const
{
    static const char* dayLongName[] =
    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    return dayLongName[m_wday];
}

// Attribute setters.
bool EasyDate::setYear(int year)
{
    if (year >= 1000 && year <= 3000)
    {
        m_year = year;
        return validateDate();
    }
    return false;
}

bool EasyDate::setMon(int mon)
{
    m_mon = mon;
    return validateDate();
}

bool EasyDate::setMDay(int mday)
{
    m_mday = mday;
    return validateDate();
}

bool EasyDate::setHour(int hour)
{
    m_hour = hour;
    return validateDate();
}

bool EasyDate::setMin(int min)
{
    m_min = min;
    return validateDate();
}

bool EasyDate::setSec(int sec)
{
    m_sec = sec;
    return validateDate();
}

bool EasyDate::setOffUtc(int offUtc)
{
    if (offUtc >= -43200 && offUtc <= 43200)
    {
        m_offUtc = offUtc;
        return true;
    }
    return false;
}

// General methods.
EasyDate EasyDate::getLocalTime() const
{
    EasyDate localTime(*this);
    localTime.setSec(localTime.getSec() + m_offUtc);
    return localTime;
}

void EasyDate::setCurrentTime()
{
    time_t now;
    struct tm lt;

    time(&now);
    lt = *gmtime(&now);

    m_year = 1900 + lt.tm_year;
    m_mon  = lt.tm_mon + 1;
    m_yday = lt.tm_yday;
    m_mday = lt.tm_mday;
    m_wday = lt.tm_wday;
    m_hour = lt.tm_hour;
    m_min  = lt.tm_min;
    m_sec  = lt.tm_sec;
    m_offUtc = -timezone;
}

bool EasyDate::setTime(int year, int mon, int mday, int hour, int min, int sec)
{
    m_year = year;
    m_mon  = mon;
    m_mday = mday;
    m_hour = hour;
    m_min  = min;
    m_sec  = sec;
    return validateDate();
}

bool EasyDate::validateDate()
{
    // Validate the seconds.
    m_min += m_sec / 60;
    m_sec = m_sec % 60;
    if (m_sec < 0)
    {
        --m_min;
        m_sec += 60;
    }

    // Validate the minutes.
    m_hour += m_min / 60;
    m_min = m_min % 60;
    if (m_min < 0)
    {
        --m_hour;
        m_min += 60;
    }

    // Validate the hours.
    m_mday += m_hour / 24;
    m_hour = m_hour % 24;
    if (m_hour < 0)
    {
        --m_mday;
        m_hour += 24;
    }

    // Validate the day of the month;
    while (m_mday > daysInMonth(m_year, m_mon))
    {
        m_mday -= daysInMonth(m_year, m_mon);
        if (++m_mon > 12)
        {
            m_mon -= 12;
            ++m_year;
        }
    }
    while (m_mday < 1)
    {
        if (--m_mon < 1)
        {
            m_mon += 12;
            --m_year;
        }
        m_mday += daysInMonth(m_year, m_mon);
    }

    // Validate the month;
    m_year += (m_mon - 1) / 12;
    m_mon = ((m_mon - 1) % 12) + 1;
    if ((m_mon - 1) < 0)
    {
        --m_year;
        m_mon += 12;
    }

    // Validate the year, this is an artificial limit.
    if (m_year < 1000 || m_year > 3000)
    {
        return false;
    }

    // Calculate the day in the year.
    m_yday = m_mday - 1;
    for (int i = 1; i < m_mon; ++i)
    {
        m_yday += daysInMonth(m_year, i);
    }

    // Help, is there an easy way?
    m_wday = m_yday;
    int i = m_year;
    while (i > 2000)
    {
        --i;
        m_wday += daysInYear(i);
    }
    while (i < 2000)
    {
        m_wday -= daysInYear(i);
        ++i;
    }
    m_wday = (m_wday + 6) % 7;
    if (m_wday < 0)
    {
        m_wday += 7;
    }

    return true;
}

int EasyDate::daysInYear(int year) const
{
    if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
    {
        return 366;
    }
    return 365;
}

int EasyDate::daysInMonth(int year, int mon) const
{
    static int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (mon == 2)
    {
        if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
        {
            return 29;
        }
    }
    return (daysInMonth[mon]);
}

bool EasyDate::validDayName(const std::string& str, int& day)
{
    static const char* dayShortName[] =
    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char* dayLongName[] =
    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    for (int i = 0; i < 7; ++i)
    {
        if (str == dayShortName[i])
        {
            day = i;
            return true;
        }
        if (str == dayLongName[i])
        {
            day = i;
            return true;
        }
    }
    return false;
}

bool EasyDate::validMonthName(const std::string& str, int& mon)
{
    static const char* monName[] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    for (int i = 0; i < 12; ++i)
    {
        if (str == monName[i])
        {
            mon = i + 1;
            return true;
        }
    }
    return false;
}

bool EasyDate::validNum(const std::string& str, int& num, int min, int max)
{
    size_t index = 0;
    int value = 0;

    while ((str[index] == ' ') && (index < str.size()))
    {
        ++index;
    }

    while ((str[index] >= '0') && (str[index] <= '9') && (index < str.size()))
    {
        value = (value * 10) + (str[index] - '0');
        ++index;
    }

    if ((index == str.size()) && (value >= min) && (value <= max))
    {
        num = value;
        return true;
    }

    return false;
}

// RFC822 and RFC1123 date format - "Sun, 06 Nov 1994 08:49:37 GMT"
std::string EasyDate::toRfc822(bool tzName) const
{
    std::ostringstream str;

    str.fill('0');
    str << getWDayName() << ", "
        << std::setw(2) << m_mday << " "
        << getMonName() << " "
        << std::setw(4) << m_year << " "
        << std::setw(2) << m_hour << ":"
        << std::setw(2) << m_min << ":"
        << std::setw(2) << m_sec << " GMT";
    return str.str();
}

// RFC822 and RFC1123 date format - "Sun, 06 Nov 1994 08:49:37 GMT"
bool EasyDate::fromRfc822(const std::string& dateStr)
{
    int year, mon, mday, hour, min, sec, wday, offHour, offMin;
    if ((dateStr.size() >= 26) &&
        validDayName(dateStr.substr(0, 3), wday) &&
        (dateStr[3] == ',') &&
        (dateStr[4] == ' ') &&
        validNum(dateStr.substr(5,  2), mday, 1, 31) &&
        (dateStr[7] == ' ') &&
        validMonthName(dateStr.substr(8, 3), mon) &&
        (dateStr[11] == ' ') &&
        validNum(dateStr.substr(12, 4), year, 1000, 3000) &&
        (dateStr[16] == ' ') &&
        validNum(dateStr.substr(17, 2), hour, 0, 23) &&
        (dateStr[19] == ':') &&
        validNum(dateStr.substr(20, 2), min, 0, 59) &&
        (dateStr[22] == ':') &&
        validNum(dateStr.substr(23, 2), sec, 0, 60) &&
        (dateStr[25] == ' '))
    {
        if ((dateStr.size() >= 31) &&
            (dateStr[26] == '+' || dateStr[26] == '-') &&
            validNum(dateStr.substr(27, 2), offHour, 0, 12) &&
            validNum(dateStr.substr(29, 2), offMin, 0, 59))
        {
            m_year   = year;
            m_mon    = mon;
            m_mday   = mday;
            m_wday   = wday;
            m_hour   = hour;
            m_min    = min;
            m_sec    = sec;
            if (dateStr[26] == '-')
                m_offUtc = -((offHour * 60) + offMin) * 60;
            else
                m_offUtc = ((offHour * 60) + offMin) * 60;
            m_sec    -= m_offUtc;
            return validateDate();
        }
        else if ((dateStr.size() >= 29) &&
                 (dateStr[26] == 'G') &&
                 (dateStr[27] == 'M') &&
                 (dateStr[28] == 'T'))
        {
            m_year   = year;
            m_mon    = mon;
            m_mday   = mday;
            m_wday   = wday;
            m_hour   = hour;
            m_min    = min;
            m_sec    = sec;
            m_offUtc = 0;
            return validateDate();
        }
    }
    return false;
}

// RFC850 & RFC1036 date format - "Sunday, 06-Nov-94 08:49:37 GMT"
std::string EasyDate::toRfc850() const
{
    std::ostringstream str;

    str.fill('0');
    str << getWDayLongName() << ", "
        << std::setw(2) << m_mday << "-"
        << getMonName() << "-"
        << std::setw(2) << (m_year % 10) << " "
        << std::setw(2) << m_hour << ":"
        << std::setw(2) << m_min << ":"
        << std::setw(2) << m_sec << " GMT";
    return str.str();
}

// RFC850 & RFC1036 date format - "Sunday, 06-Nov-94 08:49:37 GMT"
bool EasyDate::fromRfc850(const std::string& dateStr)
{
    int year, mon, mday, hour, min, sec, wday, offHour, offMin;
    size_t pos = dateStr.find(',');
    if (pos != std::string::npos)
    {
        if (validDayName(dateStr.substr(0, pos), wday) &&
            (dateStr.size() >= 21) &&
            (dateStr[0] == ',') &&
            (dateStr[1] == ' ') &&
            validNum(dateStr.substr(2,  2), mday, 1, 31) &&
            (dateStr[4] == '-') &&
            validMonthName(dateStr.substr(5, 3), mon) &&
            (dateStr[8] == '-') &&
            validNum(dateStr.substr(9, 2), year, 0, 99) &&
            (dateStr[11] == ' ') &&
            validNum(dateStr.substr(12, 2), hour, 0, 23) &&
            (dateStr[14] == ':') &&
            validNum(dateStr.substr(15, 2), min, 0, 59) &&
            (dateStr[17] == ':') &&
            validNum(dateStr.substr(18, 2), sec, 0, 60) &&
            (dateStr[20] == ' '))
        {
            if ((dateStr.size() >= 26) &&
                (dateStr[21] == '+' || dateStr[21] == '-') &&
                validNum(dateStr.substr(22, 2), offHour, 0, 12) &&
                validNum(dateStr.substr(24, 2), offMin, 0, 59))
            {
                m_year   = 1900 + year;
                m_mon    = mon;
                m_mday   = mday;
                m_wday   = wday;
                m_hour   = hour;
                m_min    = min;
                m_sec    = sec;
                if (dateStr[21] == '-')
                    m_offUtc = -((offHour * 60) + offMin) * 60;
                else
                    m_offUtc = ((offHour * 60) + offMin) * 60;
                m_sec    -= m_offUtc;
                return validateDate();
            }
            else if ((dateStr.size() >= 24) &&
                     (dateStr[21] == 'G') &&
                     (dateStr[22] == 'M') &&
                     (dateStr[23] == 'T'))
            {
                m_year   = 1900 + year;
                m_mon    = mon;
                m_mday   = mday;
                m_wday   = wday;
                m_hour   = hour;
                m_min    = min;
                m_sec    = sec;
                m_offUtc = 0;
                return validateDate();
            }
        }
    }
    return false;
}

// C library asctime() date format - "Sun Nov  6 08:49:37 1994"
std::string EasyDate::toAsctime() const
{
    std::ostringstream str;

    str.fill(' ');
    str << getWDayName() << " "
        << getMonName() << " "
        << std::setw(2) << m_mday << " ";
    str.fill('0');
    str << std::setw(2) << m_hour << ":"
        << std::setw(2) << m_min << ":"
        << std::setw(2) << m_sec << " "
        << std::setw(4) << m_year;
    return str.str();
}

// C library asctime() date format - "Sun Nov  6 08:49:37 1994"
bool EasyDate::fromAsctime(const std::string& dateStr)
{
    int year, mon, mday, hour, min, sec, wday;
    if ((dateStr.size() >= 24) &&
        validDayName(dateStr.substr(0, 3), wday) &&
        (dateStr[3] == ' ') &&
        validMonthName(dateStr.substr(4, 3), mon) &&
        (dateStr[7] == ' ') &&
        validNum(dateStr.substr(8, 2), mday, 1, 31) &&
        (dateStr[10] == ' ') &&
        validNum(dateStr.substr(11, 2), hour, 0, 23) &&
        (dateStr[13] == ':') &&
        validNum(dateStr.substr(14, 2), min, 0, 59) &&
        (dateStr[16] == ':') &&
        validNum(dateStr.substr(17, 2), sec, 0, 60) &&
        (dateStr[19] == ' ') &&
        validNum(dateStr.substr(20, 4), year, 1000, 3000))
    {
        m_year   = year;
        m_mon    = mon;
        m_mday   = mday;
        m_wday   = wday;
        m_hour   = hour;
        m_min    = min;
        m_sec    = sec;
        m_offUtc = 0;
        return validateDate();
    }
    return false;
}

// Xmltv date format - "19941106184937 +1000"
std::string EasyDate::toXmltv() const
{
    EasyDate lt = getLocalTime();
    std::ostringstream str;

    str.fill('0');
    str << std::setw(4) << lt.m_year
        << std::setw(2) << lt.m_mon
        << std::setw(2) << lt.m_mday
        << std::setw(2) << lt.m_hour
        << std::setw(2) << lt.m_min
        << std::setw(2) << lt.m_sec;
    if (m_offUtc < 0)
    {
        str << " -" << std::setw(2) << (-m_offUtc / 3600)
            << std::setw(2) << ((-m_offUtc / 60) % 60);
    }
    else
    {
        str << " +" << std::setw(2) << (m_offUtc / 3600)
            << std::setw(2) << ((m_offUtc / 60) % 60);
    }
    return str.str();
}

// Xmltv date format - "19941106184937 +1000"
bool EasyDate::fromXmltv(const std::string& dateStr)
{
    int year, mon, mday, hour, min, sec, offHour, offMin;
    if ((dateStr.size() >= 20) &&
        validNum(dateStr.substr(0, 4), year, 1000, 3000) &&
        validNum(dateStr.substr(4, 2), mon, 1, 12) &&
        validNum(dateStr.substr(6, 2), mday, 1, 31) &&
        validNum(dateStr.substr(8, 2), hour, 0, 23) &&
        validNum(dateStr.substr(10, 2), min, 0, 59) &&
        validNum(dateStr.substr(12, 2), sec, 0, 60) &&
        (dateStr[15] == '+' || dateStr[15] == '-') &&
        validNum(dateStr.substr(16, 2), offHour, 0, 12) &&
        validNum(dateStr.substr(18, 2), offMin, 0, 59))
    {
        m_year   = year;
        m_mon    = mon;
        m_mday   = mday;
        m_hour   = hour;
        m_min    = min;
        m_sec    = sec;
        if (dateStr[15] == '-')
            m_offUtc = -((offHour * 60) + offMin) * 60;
        else
            m_offUtc = ((offHour * 60) + offMin) * 60;
        m_sec    -= m_offUtc;
        return validateDate();
    }
    return false;
}

std::ostream &operator<<(std::ostream& s, const EasyDate& date)
{
    s << date.toRfc822();
    return s;
}

#if EASYDATE_UNIT_TEST
#include <iostream>

int main()
{
    EasyDate date;

    std::cout << "default date format: " << date << std::endl;
    std::cout << "RFC822 date format: " << date.toRfc822(true) << std::endl;
    std::cout << "RFC850 date format: " << date.toRfc850() << std::endl;
    std::cout << "Asctime date format: " << date.toAsctime() << std::endl;
    std::cout << "XMLTV date format: " << date.toXmltv() << std::endl;

    std::cout << std::endl;

    date.fromXmltv("19941106184937 +1000");
    std::cout << date.toXmltv() << " -- " << date.toRfc822() << std::endl;

    date.fromRfc822("Sat, 05 Nov 1995 08:49:37 GMT");
    date.setOffUtc(-9*60*60);
    std::cout << date.toXmltv() << " -- " << date.toRfc822() << std::endl;

    date.fromRfc822("Fri, 04 Nov 1996 10:49:37 +0200");
    date.setOffUtc(-2*60*60);
    std::cout << date.toXmltv() << " -- " << date.toRfc822() << std::endl;

    date.fromRfc850("Thursday, 03-Nov-97 08:49:37 GMT");
    date.setOffUtc(-1*60*60);
    std::cout << date.toXmltv() << " -- " << date.toRfc822() << std::endl;

    date.fromRfc850("Wednesday, 02-Nov-98 07:49:37 -0100");
    date.setOffUtc(1*60*60);
    std::cout << date.toXmltv() << " -- " << date.toRfc822() << std::endl;

    date.fromAsctime("Tue Nov  1 08:49:37 1999");
    date.setOffUtc(2*60*60);
    std::cout << date.toXmltv() << " -- " << date.toRfc822() << std::endl;

    return 0;
}
#endif
