//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================
#ifndef EASYDATE_HPP
#define EASYDATE_HPP

#include <string>
#include <ostream>

class EasyDate
{
public:
    EasyDate();
    EasyDate(const std::string& dateStr);
    virtual ~EasyDate();
    EasyDate(const EasyDate& copy);
    EasyDate& operator=(const EasyDate& rhs);

    // Comparison operators.
    bool operator==(const EasyDate& rhs) const;
    bool operator<(const EasyDate& rhs) const;
    bool operator>(const EasyDate& rhs) const;
    bool operator!=(const EasyDate& rhs) const { return !(*this == rhs); }
    bool operator<=(const EasyDate& rhs) const { return !(*this > rhs); }
    bool operator>=(const EasyDate& rhs) const { return !(*this < rhs); }

    // Attribute getters.
    inline int getYear() const { return m_year; }
    inline int getMon() const { return m_mon; }
    inline int getYDay() const { return m_yday; }
    inline int getMDay() const { return m_mday; }
    inline int getWDay() const { return m_wday; }
    inline int getHour() const { return m_hour; }
    inline int getMin() const { return m_min; }
    inline int getSec() const { return m_sec; }
    inline int getOffUtc() const { return m_offUtc; }
    std::string getMonName() const;
    std::string getWDayName() const;
    std::string getWDayLongName() const;

    // Attribute setters.
    bool setYear(int year);
    bool setMon(int mon);
    bool setMDay(int mday);
    bool setHour(int hour);
    bool setMin(int min);
    bool setSec(int sec);
    bool setOffUtc(int offUtc);

    // General methods.
    EasyDate getLocalTime() const;
    void setCurrentTime();
    bool setTime(int year, int mon, int mday, int hour, int min, int sec);

    // RFC822 date conversion.
    std::string toRfc822(bool tzName = false) const;
    bool fromRfc822(const std::string& dateStr);

    // RFC850 date conversion.
    std::string toRfc850() const;
    bool fromRfc850(const std::string& dateStr);

    // C lib asctime date conversion.
    std::string toAsctime() const;
    bool fromAsctime(const std::string& dateStr);

    // XMLTV date conversion.
    std::string toXmltv() const;
    bool fromXmltv(const std::string& dateStr);

private:
    bool validateDate();
    int daysInYear(int year) const;
    int daysInMonth(int year, int mon) const;
    bool validDayName(const std::string& str, int& day);
    bool validMonthName(const std::string& str, int& mon);
    bool validNum(const std::string& str, int& num, int min, int max);

    // Date and time stored as UTC, offset applied for formatting.
    int m_year;
    int m_mon;
    int m_yday;
    int m_mday;
    int m_wday;
    int m_hour;
    int m_min;
    int m_sec;
    int m_offUtc;
};

std::ostream& operator<<(std::ostream& s, const EasyDate& date);

#endif
