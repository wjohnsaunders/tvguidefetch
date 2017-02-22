//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "XmlDom.hpp"
#include "EasyDate.hpp"

class Channel
{
public:
    Channel();
    virtual ~Channel();
    Channel(const Channel &copy);
    Channel &operator=(const Channel &rhs);

    // Attribute getters.
    inline std::string getId() const { return (m_id.empty() ? getOztivoId() : m_id); }

    // Attribute setters.
    inline void setTimeOffset(int timeOffset) { m_timeOffset = timeOffset; }
    inline void setId(const std::string& str) { m_id = str; }
    inline void setIconUrl(const std::string& str) { m_iconUrl = str; }
    inline void setChanNum(const std::string& str) { m_chanNum = str; }
    inline void setDisplayname(const std::string& str) { m_displayname = str; }
    void addDisplaynameAttr(const std::string& id, const std::string& val);

    // Oztivo attribute getters and setters.
    inline std::string getOztivoId() const { return m_oztivoSource.getId(); }
    inline void setOztivoId(const std::string& str)
        { m_oztivoSource.setId(str); }
    inline void addOztivoBaseUrl(const std::string& date)
        { m_oztivoSource.addBaseUrl(date); }
    inline void addOztivoDataFor(const std::string& date)
        { m_oztivoSource.addDataFor(date); }
    inline void addOztivoDataForLastModified(const std::string& day, const std::string& lm)
        { m_oztivoSource.addDataForLastModified(day, lm); }

    // Fillin attribute getters and setters.
    inline std::string getFillinId() const { return m_fillinSource.getId(); }
    inline void setFillinId(const std::string& str) { m_fillinSource.setId(str); }
    inline void addFillinBaseUrl(const std::string& date)
        { m_fillinSource.addBaseUrl(date); }
    inline void addFillinDataFor(const std::string& date)
        { m_fillinSource.addDataFor(date); }
    inline void addFillinDataForLastModified(const std::string& day, const std::string& lm)
        { m_fillinSource.addDataForLastModified(day, lm); }

    // Clear the cache of old entries older than today.
    void clearCache(const std::string& today);

    // Fetch guide data for the range of days.
    bool fetchGuideData(const std::string& firstDay, const std::string& lastDay);

    // Serialise the channel info to an XMLTV <channel> nodes.
    void channelToXml(std::ostream &s);

    // Serialise the guide data to XMLTV <programme> nodes.
    void programsToXml(std::ostream &s, int forcedTimeZone);

private:
    class Attr
    {
    public:
        Attr(const std::string& id, const std::string& val);
        ~Attr();
        Attr(const Attr &copy);
        Attr &operator=(const Attr &copy);

        // Attribute getters and setters.
        inline std::string getId() const { return m_id; }
        inline std::string getVal() const { return m_val; }
        inline void setVal(const std::string& val) { m_val = val; }

    private:
        Attr();

        std::string m_id;
        std::string m_val;
    };

    class Source
    {
    public:
        Source();
        Source(const Source &copy);
        Source &operator=(const Source &rhs);

        // Attribute getters.
        inline std::string getId() const { return m_id; }
        inline std::vector<std::string> getBaseUrl() const { return m_baseUrl; }
        inline size_t getNumDays() const { return m_dataFor.size(); }
        inline std::string getDay(size_t i) const { return m_dataFor[i]; }
        std::string getLastModified(const std::string& day) const;

        // Attribute setters.
        inline void setId(const std::string& str) { m_id = str; }
        inline void addBaseUrl(const std::string& url) { m_baseUrl.push_back(url); }
        inline void addDataFor(const std::string& day) { m_dataFor.push_back(day); }
        inline void addDataForLastModified(const std::string& day, const std::string& lm) { m_dataForLastModified[day] = lm; }

    private:
        std::string m_id;
        std::vector<std::string> m_baseUrl;
        std::vector<std::string> m_dataFor;
        std::map<std::string, std::string> m_dataForLastModified;
    };

    class Program
    {
    public:
        Program(const XmlNav &nav);
        Program(const XmlNav &nav, const EasyDate &start, const EasyDate &stop, const std::string& name);
        Program(const Program &copy);
        Program &operator=(const Program &copy);

        // Attribute getters and setters.
        inline XmlNav getNav() const { return m_nav; }
        inline void setNav(const XmlNav &nav) { m_nav = nav; }

        inline EasyDate getStart() const { return m_start; }
        inline void setStart(const EasyDate &start) { m_start = start; }

        inline EasyDate getStop() const { return m_stop; }
        inline void setStop(const EasyDate &stop) { m_stop = stop; }

        inline std::string getName() const { return m_name; }
        inline void setName(const std::string& name) { m_name = name; }

        // Load the start and stop time attributes from the program XmlDom.
        bool loadTimes();

        // Set the timezone in the start and stop times.
        void setTimeZone(int forcedTimeZone);

        // Offset the start and stop times by the specified seconds.
        void offsetTime(int timeOffset);

        // Update the program XmlDom from the start and stop time attributes.
        bool saveTimes();

    private:
        Program();

        XmlNav m_nav;
        EasyDate m_start;
        EasyDate m_stop;
        std::string m_name;
    };

    // Add a program to the list of programs in sorted order without overlaps.
    void addProgram(Program prog);

    bool m_debugFlag;
    bool m_quietFlag;
    int m_timeOffset;
    std::string m_id;
    std::string m_iconUrl;
    std::string m_chanNum;
    std::string m_displayname;
    std::vector<Attr> m_displaynameAttr;
    Source m_oztivoSource;
    Source m_fillinSource;
    std::vector<XmlDoc> m_guideDoc;
    std::list<Program> m_programs;
};

#endif
