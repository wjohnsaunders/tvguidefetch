//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================

#include "Channel.hpp"
#include "CmdOptions.hpp"
#include "HttpFetch.hpp"

//////////////////////////////////////////////////////////////////////////////
// Channel
//////////////////////////////////////////////////////////////////////////////
Channel::Channel()
:   m_timeOffset(0)
{
    m_debugFlag = CmdParser::instance().getSwitchValue("debug");
    m_quietFlag = CmdParser::instance().getSwitchValue("quiet");
}

Channel::Channel(const Channel &copy)
:   m_debugFlag(copy.m_debugFlag),
    m_quietFlag(copy.m_quietFlag),
    m_timeOffset(copy.m_timeOffset),
    m_id(copy.m_id),
    m_iconUrl(copy.m_iconUrl),
    m_chanNum(copy.m_chanNum),
    m_displayname(copy.m_displayname),
    m_displaynameAttr(copy.m_displaynameAttr),
    m_oztivoSource(copy.m_oztivoSource),
    m_fillinSource(copy.m_fillinSource),
    m_guideDoc(copy.m_guideDoc),
    m_programs(copy.m_programs)
{
}

Channel::~Channel()
{
}

Channel &Channel::operator=(const Channel &rhs)
{
    if (this != &rhs)
    {
        m_debugFlag = rhs.m_debugFlag;
        m_quietFlag = rhs.m_quietFlag;
        m_timeOffset = rhs.m_timeOffset;
        m_id = rhs.m_id;
        m_iconUrl = rhs.m_iconUrl;
        m_chanNum = rhs.m_chanNum;
        m_displayname = rhs.m_displayname;
        m_displaynameAttr = rhs.m_displaynameAttr;
        m_oztivoSource = rhs.m_oztivoSource;
        m_fillinSource = rhs.m_fillinSource;
        m_guideDoc = rhs.m_guideDoc;
        m_programs = rhs.m_programs;
    }
    return *this;
}

void Channel::addDisplaynameAttr(const std::string& id, const std::string& val)
{
    // Replace the value of an existing id, if found.
    for (size_t i = 0; i < m_displaynameAttr.size(); ++i)
    {
        if (m_displaynameAttr[i].getId() == id)
        {
            m_displaynameAttr[i].setVal(val);
            return;
        }
    }

    // Add a new id/value pair.
    m_displaynameAttr.push_back(Attr(id, val));
}

// Clear the cache of old entries older than today.
void Channel::clearCache(const std::string& today)
{
    HttpFetch &httpFetch = HttpFetch::instance();

    std::vector<std::string> files;
    httpFetch.listCacheFiles(files);

    std::string marker(getOztivoId() + "_");
    std::string cutoff(marker + today);

    for (size_t i = 0; i < files.size(); ++i)
    {
        size_t pos = files[i].find(marker);
        if ((pos == 0) && (files[i] < cutoff))
        {
            httpFetch.removeCacheFile(files[i]);
        }
    }

    if (!getFillinId().empty())
    {
        marker = getFillinId() + "_";
        cutoff = marker + today;

        for (size_t i = 0; i < files.size(); ++i)
        {
            size_t pos = files[i].find(marker);
            if ((pos == 0) && (files[i] < cutoff))
            {
                httpFetch.removeCacheFile(files[i]);
            }

            pos = files[i].find(marker);
            if ((pos == 0) && (files[i] < cutoff))
            {
                httpFetch.removeCacheFile(files[i]);
            }
        }
    }
}

// Fetch guide data for the range of days.
bool Channel::fetchGuideData(const std::string& firstDay, const std::string& lastDay)
{
    HttpFetch &httpFetch = HttpFetch::instance();

    // Fetch data for the oztivo source.
    for (size_t i = 0; i < m_oztivoSource.getNumDays(); ++i)
    {
        std::string day(m_oztivoSource.getDay(i));

        if ((firstDay <= day) && (day <= lastDay))
        {
            std::string uri(getOztivoId() + "_" + day + ".xml.gz");
            const char *fetchMessage = 0;
            bool fetchNeeded = true;

            std::string lmDate(m_oztivoSource.getLastModified(day));
            std::string fileDate;
            if (httpFetch.getHeader(uri, "Last-Modified", fileDate))
            {
                EasyDate fileLastModified;
                EasyDate dataLastModified;
                if (dataLastModified.fromXmltv(lmDate) &&
                    fileLastModified.fromRfc822(fileDate))
                {
                    if (fileLastModified == dataLastModified)
                    {
                        fetchMessage = " (Cache Valid)";
                        fetchNeeded = false;
                    }
                }
                else
                {
                    fetchMessage = " (Fetching - invalid dates)";
                }
            }

            // Fetch the guide data for the specified day.
            if (!m_quietFlag && fetchMessage)
            {
                std::cout << uri << fetchMessage << std::endl;
            }
            if (fetchNeeded && !httpFetch.fetchFile(m_oztivoSource.getBaseUrl(), uri))
            {
                return false;
            }

            // Save the guide data in an array. This happens
            // if the file needed to be fetched or not.
            std::string contents;
            if (!httpFetch.getFileContents(uri, contents))
            {
                return false;
            }

            // Parse the XML document.
            XmlDoc doc;
            if (doc.parse(contents))
            {
                m_guideDoc.push_back(doc);
                XmlNav nav(doc);
                if (nav.gotoNode("/tv"))
                {
                    bool progSuccess = nav.gotoFirstChildNode();
                    while (progSuccess)
                    {
                        if (nav.gotoAttr("channel"))
                        {
                            nav.setAttrValue(getId().c_str());
                        }
                        Program prog(nav);
                        if (prog.loadTimes())
                        {
                            addProgram(prog);
                        }
                        progSuccess = nav.gotoNextSiblingNode();
                    }
                }
            }
        }
    }

    // Fetch data for the fillin source.
    for (size_t i = 0; i < m_fillinSource.getNumDays(); ++i)
    {
        std::string day(m_fillinSource.getDay(i));

        if ((firstDay <= day) && (day <= lastDay))
        {
            std::string uri(getFillinId() + "_" + day + ".xml.gz");
            const char *fetchMessage = 0;
            bool fetchNeeded = true;

            std::string lmDate(m_fillinSource.getLastModified(day));
            std::string fileDate;
            if (httpFetch.getHeader(uri, "Last-Modified", fileDate))
            {
                EasyDate fileLastModified;
                EasyDate dataLastModified;
                if (dataLastModified.fromXmltv(lmDate) &&
                    fileLastModified.fromRfc822(fileDate))
                {
                    if (fileLastModified == dataLastModified)
                    {
                        fetchMessage = " (Not modified)";
                        fetchNeeded = false;
                    }
                }
                else
                {
                    fetchMessage = " (Fetching - invalid dates)";
                }
            }

            // Fetch the guide data for the specified day.
            if (!m_quietFlag && fetchMessage)
            {
                std::cout << uri << fetchMessage << std::endl;
            }
            if (fetchNeeded && !httpFetch.fetchFile(m_fillinSource.getBaseUrl(), uri))
            {
                return false;
            }

            // Save the guide data in an array. This happens
            // if the file needed to be fetched or not.
            std::string contents;
            if (!httpFetch.getFileContents(uri, contents))
            {
                return false;
            }

            // Parse the XML document.
            XmlDoc doc;
            if (doc.parse(contents))
            {
                m_guideDoc.push_back(doc);
                XmlNav nav(doc);
                if (nav.gotoNode("/tv"))
                {
                    bool progSuccess = nav.gotoFirstChildNode();
                    while (progSuccess)
                    {
                        if (nav.gotoAttr("channel"))
                        {
                            nav.setAttrValue(getId().c_str());
                        }
                        Program prog(nav);
                        if (prog.loadTimes())
                        {
                            addProgram(prog);
                        }
                        progSuccess = nav.gotoNextSiblingNode();
                    }
                }
            }
        }
    }

    return true;
}

// Add a program to the list of programs in sorted order without overlaps.
void Channel::addProgram(Program prog)
{
    std::list<Program>::iterator i;
    bool changed = false;

    for (i = m_programs.begin(); i != m_programs.end(); ++i)
    {
        //
        // Case 1 is when the period the new program runs for
        // is completed allocated to an existing program.
        //
        if (prog.getStart() >= i->getStart() &&
            prog.getStop() <= i->getStop())
        {
            if (m_debugFlag)
            {
                std::cout << "skipping    "
                        << " " << prog.getStart().toXmltv()
                        << "->" << prog.getStop().toXmltv()
                        << " " << prog.getName() << std::endl;
            }
            return;
        }
        //
        // Case 2 is where the new program stradles an existing
        // program. In this case we fragment the new program into 2.
        //
        else if (prog.getStart() < i->getStart() &&
                 prog.getStop() > i->getStop())
        {
            if (m_debugFlag)
            {
                std::cout << "split part1 "
                        << " " << prog.getStart().toXmltv()
                        << "->" << i->getStart().toXmltv()
                        << " " << prog.getName() << std::endl;
                std::cout << "split part2 "
                        << " " << i->getStop().toXmltv()
                        << "->" << prog.getStop().toXmltv()
                        << " " << prog.getName() << std::endl;
            }
            addProgram(Program(prog.getNav(), prog.getStart(),
                               i->getStart(), prog.getName()));
            prog.setStart(i->getStop());
            changed = true;
        }
        //
        // Case 3 is where the start of the new program is
        // allocated to an existing program.
        //
        else if (prog.getStart() >= i->getStart() &&
                 prog.getStart() < i->getStop() &&
                 prog.getStop() > i->getStop())
        {
            if (m_debugFlag)
            {
                std::cout << "move start  "
                        << " " << i->getStop().toXmltv()
                        << "->" << prog.getStop().toXmltv()
                        << " " << prog.getName() << std::endl;
            }
            prog.setStart(i->getStop());
            changed = true;
        }
        //
        // Case 4 is where the end of the new program is
        // allocated to an existing program.
        //
        else if (prog.getStart() < i->getStart() &&
                 prog.getStop() > i->getStart() &&
                 prog.getStop() <= i->getStop())
        {
            if (m_debugFlag)
            {
                std::cout << "move stop   "
                        << " " << prog.getStart().toXmltv()
                        << "->" << i->getStart().toXmltv()
                        << " " << prog.getName() << std::endl;
            }
            prog.setStop(i->getStart());
            changed = true;
        }
    }

    if (!changed && m_debugFlag)
    {
        std::cout << "keeping     "
                << " " << prog.getStart().toXmltv()
                << "->" << prog.getStop().toXmltv()
                << " " << prog.getName()
                << std::endl;
    }

    // All clashes resolved, insert the new program.
    for (i = m_programs.begin(); i != m_programs.end(); ++i)
    {
        if (prog.getStart() < i->getStart())
        {
            m_programs.insert(i, prog);
            break;
        }
    }
    if (i == m_programs.end())
    {
        m_programs.insert(i, prog);
    }
}

// Serialise the channel info to an XMLTV <channel> nodes.
void Channel::channelToXml(std::ostream &s)
{
    s << "  <channel id=\"" << XmlDoc::quoteString(getId()) << "\">" << std::endl;
    s << "    <display-name";
    for (size_t i = 0; i < m_displaynameAttr.size(); ++i)
    {
        s << " " << XmlDoc::quoteString(m_displaynameAttr[i].getId()) <<
            "=\"" << XmlDoc::quoteString(m_displaynameAttr[i].getVal()) << "\"";
    }
    s << ">" << XmlDoc::quoteString(m_displayname) << "</display-name>" << std::endl;
    if (!m_chanNum.empty())
    {
        s << "    <display-name>" << m_chanNum << "</display-name>" << std::endl;
    }
    if (!m_iconUrl.empty())
    {
        s << "    <icon src=\"" << m_iconUrl << "\"/>" << std::endl;
    }
    s << "  </channel>" << std::endl;
}

// Serialise the guide data to XMLTV <programme> nodes.
void Channel::programsToXml(std::ostream &s, int forcedTimeZone)
{
    std::list<Program>::iterator i;

    for (i = m_programs.begin(); i != m_programs.end(); ++i)
    {
        i->setTimeZone(forcedTimeZone);
        i->offsetTime(m_timeOffset);
        if (i->saveTimes())
        {
            s << std::endl << i->getNav() << std::endl;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// Channel::Attr
//////////////////////////////////////////////////////////////////////////////
Channel::Attr::Attr(const std::string& id, const std::string& val)
:   m_id(id),
    m_val(val)
{
}

Channel::Attr::Attr(const Attr &copy)
:   m_id(copy.m_id),
    m_val(copy.m_val)
{
}

Channel::Attr::~Attr()
{
}

Channel::Attr &Channel::Attr::operator=(const Attr &rhs)
{
    if (this != &rhs)
    {
        m_id = rhs.m_id;
        m_val = rhs.m_val;
    }
    return *this;
}

//////////////////////////////////////////////////////////////////////////////
// Channel::Source
//////////////////////////////////////////////////////////////////////////////
Channel::Source::Source()
{
}

Channel::Source::Source(const Source &copy)
:   m_id(copy.m_id),
    m_baseUrl(copy.m_baseUrl),
    m_dataFor(copy.m_dataFor),
    m_dataForLastModified(copy.m_dataForLastModified)
{
}

Channel::Source &Channel::Source::operator=(const Source &rhs)
{
    if (this != &rhs)
    {
        m_id = rhs.m_id;
        m_baseUrl = rhs.m_baseUrl;
        m_dataFor = rhs.m_dataFor;
        m_dataForLastModified = rhs.m_dataForLastModified;
    }
    return *this;
}

std::string Channel::Source::getLastModified(const std::string& day) const
{
    std::map<std::string, std::string>::const_iterator iter;
    if ((iter = m_dataForLastModified.find(day)) != m_dataForLastModified.end())
    {
        return iter->second;
    }
    return std::string("");
}

//////////////////////////////////////////////////////////////////////////////
// Channel::Program
//////////////////////////////////////////////////////////////////////////////
Channel::Program::Program(const XmlNav &nav)
:   m_nav(nav)
{
}

Channel::Program::Program(const XmlNav &nav, const EasyDate &start,
        const EasyDate &stop, const std::string& name)
:   m_nav(nav),
    m_start(start),
    m_stop(stop),
    m_name(name)
{
}

Channel::Program::Program(const Program &copy)
:   m_nav(copy.m_nav),
    m_start(copy.m_start),
    m_stop(copy.m_stop),
    m_name(copy.m_name)
{
}

Channel::Program &Channel::Program::operator=(const Program &copy)
{
    if (this != &copy)
    {
        m_nav = copy.m_nav;
        m_start = copy.m_start;
        m_stop = copy.m_stop;
        m_name = copy.m_name;
    }
    return *this;
}

// Load the start and stop time attributes from the program XmlDom.
bool Channel::Program::loadTimes()
{
    if (!m_nav.gotoAttr("start"))
    {
        return false;
    }
    m_start.fromXmltv(m_nav.getAttrValue());

    if (!m_nav.gotoAttr("stop"))
    {
        return false;
    }
    m_stop.fromXmltv(m_nav.getAttrValue());

    XmlNav newNav(m_nav);
    if (!newNav.gotoChildNode("title"))
    {
        return false;
    }
    m_name = newNav.getAllChars();

    return true;
}

// Adjust the timezone in the start and stop times.
void Channel::Program::setTimeZone(int forcedTimeZone)
{
    int tz = m_start.getOffUtc();
    if (m_start.setOffUtc(forcedTimeZone))
    {
        m_start.setSec(m_start.getSec() + tz);
        m_start.setSec(m_start.getSec() - forcedTimeZone);
    }
    tz = m_stop.getOffUtc();
    if (m_stop.setOffUtc(forcedTimeZone))
    {
        m_stop.setSec(m_stop.getSec() + tz);
        m_stop.setSec(m_stop.getSec() - forcedTimeZone);
    }
}

// Offset the start and stop times by the specified seconds.
void Channel::Program::offsetTime(int timeOffset)
{
    m_start.setSec(m_start.getSec() + timeOffset);
    m_stop.setSec(m_stop.getSec() + timeOffset);
}

// Update the program XmlDom from the start and stop time attributes.
bool Channel::Program::saveTimes()
{
    if (!m_nav.gotoAttr("start"))
    {
        return false;
    }
    m_nav.setAttrValue(m_start.toXmltv().c_str());

    if (!m_nav.gotoAttr("stop"))
    {
        return false;
    }
    m_nav.setAttrValue(m_stop.toXmltv().c_str());

    return true;
}
