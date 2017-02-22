//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>

#include "Version.hpp"
#include "XmlDom.hpp"
#include "CmdOptions.hpp"
#include "EasyDate.hpp"
#include "TvGuideFetch.hpp"

int main(int argc, char **argv)
{
    TvGuideFetch fetch;

    return fetch.main(argc, argv);
}

//////////////////////////////////////////////////////////////////////////////
// TvGuideFetch
//////////////////////////////////////////////////////////////////////////////

TvGuideFetch::TvGuideFetch()
:   m_quietFlag(0),
    m_outputPath(0),
    m_daysToFetch(14),
    m_offsetFromToday(0),
    m_configFilePath("tvguide_config.xml"),
    m_debugFlag(0),
    m_forcedTimeZone(99999), // out-of-range
    m_cmdParser(CmdParser::instance()),
    m_config(Config::instance()),
    m_httpFetch(HttpFetch::instance())
{
}

TvGuideFetch::~TvGuideFetch()
{
}

// Main entry point.
int TvGuideFetch::main(int argc, char **argv)
{
    const char *daysString = 0;
    const char *offsetString = 0;
    const char *forcedTimeZoneString = 0;
    unsigned char configureFlag = 0;

    // Add a help option.
    m_cmdParser.addMethodOption('h', "help", "Print this message and exit.", &printUsage);
    // Add standard options.
    m_cmdParser.addMethodOption('v', "version", "Display the version number and exit.", &printVersion);
    m_cmdParser.addMethodOption(0, "description", "Tell everyone that we\'re an Australian grabber.", &printDescription);
    m_cmdParser.addMethodOption(0, "capabilities", "List XMLTV capabilities.", &printCapabilities);
    // Add baseline options.
    m_cmdParser.addSwitchOption(0, "quiet", "Suppress all progress information.", &m_quietFlag);
    m_cmdParser.addArgumentOption('o', "output", "Output to <FILE> rather than standard output.", &m_outputPath);
    m_cmdParser.addArgumentOption('d', "days", "Grab <N> days. Defaults to grabbing 7 days of guide data.", &daysString);
    m_cmdParser.addArgumentOption(0, "offset", "Start grabbing <N> days in the future. Defaults to 0; starting grabbing with today\'s data.", &offsetString);
    // Add manualconfig options.
    m_cmdParser.addSwitchOption(0, "configure", "Write a default configuration file. This file will need to be edited with appropriate information.", &configureFlag);
    m_cmdParser.addArgumentOption('c', "config-file", "Read configuration information from <FILE> instead of from config.xml.", &m_configFilePath);
    // Add preferredmethod options.
    m_cmdParser.addMethodOption(0, "preferredmethod", "Tell the calling program that we prefer to return all the data at once.", &printPreferredMethod);
    // Add other options.
    m_cmdParser.addSwitchOption(0, "debug", "Display diagnostic information as we go about our business.", &m_debugFlag);
    m_cmdParser.addArgumentOption(0, "timezone", "Specify a timezone in minutes to override the oztivo timezone.", &forcedTimeZoneString);

    // Parse the command line.
    if (!m_cmdParser.parse(argc, argv))
    {
        m_cmdParser.writeCmdHelp(std::cerr);
        return EXIT_FAILURE;
    }

    // Ensure no unwanted parameters.
    if (m_cmdParser.getParameter(0) != 0)
    {
        std::cerr << m_cmdParser.getProgramName() << ": unwanted parameters:";
        for (int i = 0; m_cmdParser.getParameter(i); ++i)
            std::cerr << " '" << m_cmdParser.getParameter(i) << "'";
        std::cerr << std::endl;
        m_cmdParser.writeCmdHelp(std::cerr);
        return EXIT_FAILURE;
    }

    // Convert integer arguments.
    if (daysString && !validPositiveInt(daysString, m_daysToFetch))
    {
        std::cerr << m_cmdParser.getProgramName() <<
                ": invalid days: " << daysString << std::endl;
        m_cmdParser.writeCmdHelp(std::cerr);
        return EXIT_FAILURE;
    }
    if (offsetString && !validPositiveInt(offsetString, m_offsetFromToday))
    {
        std::cerr << m_cmdParser.getProgramName() <<
                ": invalid offset: " << offsetString << std::endl;
        m_cmdParser.writeCmdHelp(std::cerr);
        return EXIT_FAILURE;
    }
    if (forcedTimeZoneString && !validInt(forcedTimeZoneString, m_forcedTimeZone, -720, 720))
    {
        std::cerr << m_cmdParser.getProgramName() <<
                ": invalid timezone offset: " << forcedTimeZoneString << std::endl;
        m_cmdParser.writeCmdHelp(std::cerr);
        return EXIT_FAILURE;
    }

    // Display the state of command switches.
    if (m_debugFlag)
    {
        std::cout << "Running with the following options:" << std::endl;
        std::cout << "Quiet flag: " << (m_quietFlag ? "TRUE" : "FALSE") << std::endl;
        std::cout << "Output path: " << (m_outputPath ? m_outputPath : "<stdout>") << std::endl;
        std::cout << "Days to fetch: " << m_daysToFetch << std::endl;
        std::cout << "Offset from today: " << m_offsetFromToday << std::endl;
        std::cout << "Forced timezone: " << m_forcedTimeZone << std::endl;
        std::cout << "Config-file path: " << m_configFilePath << std::endl;
    }

    if (configureFlag)
    {
        // Write a default configuration when run with --configure.
        printDefaultConfig();
    }
    else
    {
        // Establish the configuration location.
        if (!m_config.setConfigPath(m_configFilePath))
        {
            bailOut("config: cannot load specified config file.");
        }

        // Write the XMLTV guide data.
        if (m_outputPath)
        {
            std::ofstream out;
            out.open(m_outputPath, std::ios_base::out | std::ios_base::trunc);
            if (!out.is_open())
            {
                return EXIT_FAILURE;
            }
            printGuideData(out);
            out.close();
        }
        else
        {
            printGuideData(std::cout);
        }
    }

    return EXIT_SUCCESS;
}

// Method called to print the grabber version.
void TvGuideFetch::printVersion(CmdParser& parser)
{
    std::cout << NAME << " version " << VERSION << std::endl;
    exit(EXIT_SUCCESS);
}

// Method called to print the grabber description.
void TvGuideFetch::printDescription(CmdParser& parser)
{
    std::cout << NAME << " (" << DESCRIPTION << ")" << std::endl;
    exit(EXIT_SUCCESS);
}

// Method called to print the grabber capabilities.
void TvGuideFetch::printCapabilities(CmdParser& parser)
{
    std::cout << "baseline" << std::endl;
    std::cout << "manualconfig" << std::endl;
    std::cout << "preferredmethod" << std::endl;
    exit(EXIT_SUCCESS);
}

// Method called to print the preferred access method.
void TvGuideFetch::printPreferredMethod(CmdParser& parser)
{
    std::cout << "allatonce" << std::endl;
    exit(EXIT_SUCCESS);
}

// Method called for command line help/errors.
void TvGuideFetch::printUsage(CmdParser& parser)
{
    std::cout << "usage: " << parser.getProgramName() << " option(s)..." << std::endl;

    parser.writeCmdHelp(std::cout);

    exit(EXIT_SUCCESS);
}

// Print a default configuration as a starting point.
void TvGuideFetch::printDefaultConfig()
{
    std::cout << NAME << " version " << VERSION
              << " Configuration Wizard" << std::endl;

    // If we can successfully load the config, then don't overwrite it.
    if (m_config.setConfigPath(m_configFilePath))
    {
        std::cout << "Config \"" << m_configFilePath << "\" already exists." << std::endl
                  << "Enter \"yes\" to overwrite and re-configure -> ";
        std::string response = readLine();
        if (response != "yes" && response != "YES" && response != "Yes")
        {
            if (!m_quietFlag)
            {
                std::cout << "config: Config \"" << m_configFilePath << "\" already exists, skipping re-configure" << std::endl;
            }
            exit(EXIT_FAILURE);
        }
    }

    // Load the base listings file to customise from.
    std::cout << "Select the channel lineup for this video source:" << std::endl
              << "Empty, Freeview, Analogue, Foxtel -> ";
    std::string listingsType = readLine();

    // Load the internal default config (shouldn't fail).
    if (!m_config.loadDefaultConfig(listingsType))
    {
        bailOut("config: Cannot load specified default config!");
    }
    else
    {
        std::cout << "Your default guide data cache is as follows:" << std::endl
                  << "  \"" << cacheDirectory() << "\"" << std::endl
                  << "press Enter to accept, or provide an alternative -> ";
        std::string cachePath = readLine();
        if (cachePath.empty())
        {
            cachePath = cacheDirectory();
        }
        if (!m_config.setItem("/config/connection/cache-path", cachePath))
        {
            bailOut("config: Cannot change <cache-path> setting!");
        }

        std::cout << "Enter your oztivo username -> ";
        std::string username = readLine();
        if (!m_config.setItem("/config/connection/username", username))
        {
            bailOut("config: Cannot change <username> setting!");
        }

        std::cout << "Enter your oztivo password -> ";
        std::string password = readLine();
        if (!m_config.setItem("/config/connection/password", password))
        {
            bailOut("config: Cannot change <password> setting!");
        }

        if (!m_config.writeConfig())
        {
            bailOut("config: Cannot write default config file!");
        }
    }

    if (!m_quietFlag)
    {
        std::cout << "config: Created \"" << m_configFilePath << "\" successfully!" << std::endl;
    }
}

// Read a line and strip trailing return characters.
std::string TvGuideFetch::readLine()
{
    char buffer[4096];

    // Read up to the next line-feed character.
    std::cin.getline(buffer, sizeof(buffer));
    std::string line(buffer);

    // Strip return characters, if present.
    size_t pos = line.find('\r');
    if (pos != std::string::npos)
    {
        line = line.substr(0, pos);
    }

    return line;
}

// Formulate the default cache directory path.
std::string TvGuideFetch::cacheDirectory()
{
    const char *home = getenv("HOME");
    if (home == 0)
    {
        return std::string("/var/tmp/tvguide_cache");
    }
    return std::string(home) + "/.xmltv/tvguide_cache";
}

// Construct and print the XMLTV guide data.
void TvGuideFetch::printGuideData(std::ostream& out)
{
    initConfiguration();

    // Fetch the list of channels and available guide data.
    if (!m_quietFlag)
    {
        std::cout << "Processing oztivo channel+data list" << std::endl;
    }
    XmlDoc datalist;
    fetchDatalist(datalist);
    XmlNav dataNav(datalist);

    // Fetch the list of configured channels.
    if (!m_quietFlag)
    {
        std::cout << "Processing configured channel list" << std::endl;
    }
    XmlNav chanNav = m_config.getConfigNav();
    if (chanNav.gotoNode("/config/channelmap"))
    {
        bool success = chanNav.gotoFirstChildNode();
        while (success)
        {
            Channel chan;

            if (chanNav.gotoAttr("oztivoid"))
            {
                if (m_debugFlag)
                {
                    std::cout << "debug: Found channel in config - " << chanNav.getAttrValue() << std::endl;
                }

                // Ensure the oztivoid specifies an existing channel.
                if (findChannel(dataNav, chanNav.getAttrValue()))
                {
                    if (m_debugFlag)
                    {
                        std::cout << "debug: Found channel in datalist - " << chanNav.getAttrValue() << std::endl;
                    }

                    // Populate the channel with the oztivo data.
                    if (populateSettings(dataNav, chan) &&
                        populateSource(dataNav, chan))
                    {
                        if (m_debugFlag)
                        {
                            std::cout << "debug: Got channel data from datalist - " << chanNav.getAttrValue() << std::endl;
                        }

                        // Replace the channel with overrides from our configuration.
                        if (populateSettings(chanNav, chan))
                        {
                            if (m_debugFlag)
                            {
                                std::cout << "debug: Overwriting channel data from config - " << chanNav.getAttrValue() << std::endl;
                            }

                            if (chanNav.gotoAttr("fillinid"))
                            {
                                // Ensure the fillinid specifies an existing channel.
                                if (findChannel(dataNav, chanNav.getAttrValue()))
                                {
                                    if (m_debugFlag)
                                    {
                                        std::cout << "debug: Found channel in datalist - " << chanNav.getAttrValue() << std::endl;
                                    }

                                    // Populate the channel with the fillin data.
                                    if (populateSource(dataNav, chan, true))
                                    {
                                        if (m_debugFlag)
                                        {
                                            std::cout << "debug: Got channel data from datalist - " << chanNav.getAttrValue() << std::endl;
                                        }
                                    }
                                    else
                                    {
                                        bailOut("datalist: not correctly positioned on a <channel> node.");
                                    }
                                }
                                else
                                {
                                    std::ostringstream msg;
                                    msg << "warning: <channel> \"fillinid\" attribute \"" << chanNav.getAttrValue() << "\" doesn't exist!";
                                    warnUser(msg.str());
                                }
                            }

                            m_channels.push_back(chan);
                        }
                        else
                        {
                            bailOut("config: not correctly positioned on a <channel> node.");
                        }
                    }
                    else
                    {
                        bailOut("datalist: not correctly positioned on a <channel> node.");
                    }
                }
                else
                {
                    std::ostringstream msg;
                    msg << "warning: <channel> \"oztivoid\" attribute \"" << chanNav.getAttrValue() << "\" doesn't exist!";
                    warnUser(msg.str());
                }
            }
            else
            {
                bailOut("config: <channel> must contain an \"oztivoid\" attribute.");
            }
            success = chanNav.gotoNextSiblingNode();
        }
        if (m_channels.size() == 0)
        {
            warnUser("config: No valid <channel> nodes defined in <channelmap>.");
        }
    }
    else
    {
        bailOut("config: No <channelmap> node defined in <config>.");
    }

    // Find the guide data date range to fetch.
    std::string today(getDate(0));
    std::string firstDay(getDate(m_offsetFromToday));
    std::string lastDay(getDate(m_offsetFromToday + m_daysToFetch - 1));
    if (m_debugFlag)
    {
        std::cout << "debug: Guide data date range - " << firstDay <<
                " to " << lastDay << " (today " << today << ")" << std::endl;
    }

    // Process all of the channels.
    for (size_t chan = 0; chan < m_channels.size(); ++chan)
    {
        // Provide a progress message.
        if (!m_quietFlag)
        {
            std::cout << "Fetching guide data for channel (" <<
                    (chan + 1) << " of " << m_channels.size() <<
                    ") " << m_channels[chan].getId() << std::endl;
        }

        // Tell the channel to clear old entries from the cache.
        m_channels[chan].clearCache(today);

        // Tell the channel to fetch the guide data.
        if (!m_channels[chan].fetchGuideData(firstDay, lastDay))
        {
            bailOut("http: Error fetching guide data");
        }
    }

    if (!m_quietFlag)
    {
        std::cout << "Generating xmltv output file" << std::endl;
    }

    // Output the XMLTV header.
    out << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << std::endl;
    out << "<!DOCTYPE tv SYSTEM \"xmltv.dtd\">" << std::endl << std::endl;
    out << "<tv generator-info-name=\"TvGuideFetch version 0.1\">" << std::endl;

    // Output the XMLTV channels.
    for (size_t chan = 0; chan < m_channels.size(); ++chan)
    {
        m_channels[chan].channelToXml(out);
    }

    // Output the XMLTV programmes.
    for (size_t chan = 0; chan < m_channels.size(); ++chan)
    {
        m_channels[chan].programsToXml(out, m_forcedTimeZone * 60);
    }

    // Output the XMLTV footer.
    out << "</tv>" << std::endl;

    if (!m_quietFlag)
    {
        std::cout << "All done." << std::endl;
    }
}

// Initialise the configuration.
void TvGuideFetch::initConfiguration()
{
    if (!m_quietFlag)
    {
        std::cout << "Loading config-file \"" << m_configFilePath << "\"" << std::endl;
    }

    m_httpFetch.setQuietFlag(m_quietFlag);

    std::string cachePath;
    if (!m_config.getItem("/config/connection/cache-path", cachePath))
    {
        bailOut("config: <cache-path> setting missing from <connection>.");
    }
    m_httpFetch.setCachePath(cachePath);

    std::string username;
    if (!m_config.getItem("/config/connection/username", username))
    {
        bailOut("config: <username> setting missing from <connection>.");
    }
    m_httpFetch.setUsername(username.c_str());

    std::string password;
    if (!m_config.getItem("/config/connection/password", password))
    {
        bailOut("config: <password> setting missing from <connection>.");
    }
    m_httpFetch.setPassword(password.c_str());
}

// Fetch the list of channels and valid days.
void TvGuideFetch::fetchDatalist(XmlDoc& datalist)
{
    std::vector<std::string> baseUrls;
    if (!m_config.getItemList("/config/connection", "base-url", baseUrls))
    {
        bailOut("config: <base-url> setting missing from <connection>.");
    }

    std::string datalistUri;
    if (!m_config.getItem("/config/connection/datalist", datalistUri))
    {
        bailOut("config: <datalist> setting missing from <connection>.");
    }

    if (!m_httpFetch.fetchFile(baseUrls, datalistUri))
    {
        std::stringstream msg;
        msg << "http: Fetch of " << datalistUri << " from";
        for (size_t i = 0; i < baseUrls.size(); ++i)
        {
            msg << (i == 0 ? " " : ", ") << baseUrls[i];
        }
        msg << " failed - " << m_httpFetch.getErrorMessage();
        bailOut(msg.str().c_str());
    }

    std::string contents;
    if (!m_httpFetch.getFileContents(datalistUri, contents))
    {
        std::stringstream msg;
        msg << "http: Get contents of " << datalistUri << " failed!";
        bailOut(msg.str().c_str());
    }

    if (!datalist.parse(contents))
    {
        std::stringstream msg;
        msg << "xml: Cannot parse contents of " << datalistUri << "!";
        bailOut(msg.str().c_str());
    }
}

// Find a channel with the specified id.
bool TvGuideFetch::findChannel(XmlNav& nav, const std::string& id)
{
    if (nav.gotoNode("/tv"))
    {
        bool success = nav.gotoFirstChildNode();
        while (success)
        {
            if (std::string(nav.getNodeName()) == "channel")
            {
                if (nav.gotoAttr("id") && (id == nav.getAttrValue()))
                {
                    return true;
                }
            }
            success = nav.gotoNextSiblingNode();
        }
    }

    return false;
}

// Fill in the channel with settings from the config.
bool TvGuideFetch::populateSettings(XmlNav nav, Channel& chan)
{
    std::string nodeName(nav.getNodeName());
    if (nodeName == "channel")
    {
        // Populate the channel with the known attribute values.
        bool success = nav.gotoFirstAttr();
        while (success)
        {
            std::string attrName(nav.getAttrName());
            if (attrName == "id")
            {
                chan.setId(nav.getAttrValue());
            }
            else if (attrName == "oztivoid")
            {
                chan.setOztivoId(nav.getAttrValue());
            }
            else if (attrName == "fillinid")
            {
                chan.setFillinId(nav.getAttrValue());
            }
            else if (attrName == "timeoffset")
            {
                int timeOffset;
                if (validInt(nav.getAttrValue().c_str(), timeOffset, -43200, 43200))
                {
                    chan.setTimeOffset(timeOffset);
                }
                else
                {
                    bailOut("config: <channel> \"timeoffset\" attribute not in the range -43200 to 43200.");
                }
            }

            success = nav.gotoNextAttr();
        }

        // Populate the channel with the known child node values.
        bool seenLocation = false;
        success = nav.gotoFirstChildNode();
        while (success)
        {
            nodeName.assign(nav.getNodeName());
            if (nodeName == "display-name")
            {
                // Add all of the display-name attributes.
                bool success = nav.gotoFirstAttr();
                while (success)
                {
                    chan.addDisplaynameAttr(nav.getAttrName(), nav.getAttrValue());
                    success = nav.gotoNextAttr();
                }

                // Add the display-name text.
                chan.setDisplayname(nav.getAllChars());
            }
            else if (nodeName == "lcn" && !seenLocation)
            {
                // Add the first channel number.
                chan.setChanNum(nav.getAllChars());
                seenLocation = true;
            }
            else if (nodeName == "icon")
            {
                // Add the src attribute.
                if (nav.gotoAttr("src"))
                {
                    chan.setIconUrl(nav.getAttrValue());
                }
            }

            success = nav.gotoNextSiblingNode();
        }

        return true;
    }

    return false;
}

// Fill in the channel with data from the source channel.
bool TvGuideFetch::populateSource(XmlNav nav, Channel& chan, bool fillin)
{
    std::string nodeName(nav.getNodeName());
    if (nodeName == "channel")
    {
        // Populate the channel with the known child node values.
        bool success = nav.gotoFirstChildNode();
        while (success)
        {
            nodeName.assign(nav.getNodeName());
            if (nodeName == "base-url")
            {
                // Add the base url text.
                if (!fillin)
                    chan.addOztivoBaseUrl(nav.getAllChars());
                else
                    chan.addFillinBaseUrl(nav.getAllChars());
            }
            else if (nodeName == "datafor")
            {
                std::string day = nav.getAllChars();

                // Add the lastmodified attribute.
                if (nav.gotoAttr("lastmodified"))
                {
                    if (!fillin)
                        chan.addOztivoDataForLastModified(day, nav.getAttrValue());
                    else
                        chan.addFillinDataForLastModified(day, nav.getAttrValue());
                }

                // Add the datafor text.
                if (!fillin)
                    chan.addOztivoDataFor(day);
                else
                    chan.addFillinDataFor(day);
            }

            success = nav.gotoNextSiblingNode();
        }

        return true;
    }

    return false;
}

// Convert string to integer with strict error checking.
bool TvGuideFetch::validPositiveInt(const std::string& str, int& num)
{
    size_t index = 0;
    if (index < str.size())
    {
        int value = 0;

        while ((index < str.size()) && (str[index] >= '0') && (str[index] <= '9'))
        {
            value = (value * 10) + (str[index] - '0');
            ++index;
        }

        if (str.size() == index)
        {
            num = value;
            return true;
        }
    }

    return false;
}

// Convert string to integer with range check.
bool TvGuideFetch::validInt(const std::string& str, int& num, int min, int max)
{
    size_t index = 0;
    if (index < str.size())
    {
        int sign = 1;

        if ((index < str.size()) && ((str[index] == '-') || (str[index] == '+')))
        {
            sign = (str[index] == '+') ? 1 : -1;
            ++index;
        }

        if (index < str.size())
        {
            int value = 0;

            while ((index < str.size()) && (str[index] >= '0') && (str[index] <= '9'))
            {
                value = (value * 10) + (str[index] - '0');
                ++index;
            }

            if ((str.size() == index) && (value >= min) && (value <= max))
            {
                num = value * sign;
                return true;
            }
        }
    }

    return false;
}

// Get the date string YYYY-MM-DD at the offset from today.
std::string TvGuideFetch::getDate(int offset)
{
    EasyDate theTime;
    EasyDate localTime = theTime.getLocalTime();

    if (offset != 0)
    {
        // Advance the specified number of days.
        localTime.setMDay(localTime.getMDay() + offset);

        if (m_debugFlag)
        {
            std::cout << "Time with " << offset << " days offset is " << localTime << std::endl;
        }
    }
    else
    {
        if (m_debugFlag)
        {
            std::cout << "Current time is " << localTime << std::endl;
        }
    }

    // Format the date as YYYY-MM-DD.
    std::ostringstream str;
    str.fill('0');
    str << std::setw(4) << localTime.getYear() << "-"
        << std::setw(2) << localTime.getMon() << "-"
        << std::setw(2) << localTime.getMDay();
    return str.str();
}

// Display a warning message and exit quickly.
void TvGuideFetch::warnUser(const std::string& msg)
{
    std::cerr << msg << std::endl;
}

// Display an error message and exit quickly.
void TvGuideFetch::bailOut(const std::string& msg)
{
    std::cerr << msg << std::endl;
    exit(EXIT_FAILURE);
}
