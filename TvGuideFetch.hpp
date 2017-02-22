//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================
#ifndef TVGUIDEFETCH_HPP
#define TVGUIDEFETCH_HPP

#include <vector>

#include "Config.hpp"
#include "HttpFetch.hpp"
#include "Channel.hpp"

class TvGuideFetch
{
public:
    TvGuideFetch();
    ~TvGuideFetch();

    // Main entry point.
    int main(int argc, char **argv);

private:
    // Method called to print the grabber version.
    static void printVersion(CmdParser& parser);

    // Method called to print the grabber description.
    static void printDescription(CmdParser& parser);

    // Method called to print the grabber capabilities.
    static void printCapabilities(CmdParser& parser);

    // Method called to print the preferred access method.
    static void printPreferredMethod(CmdParser& parser);

    // Method called for command line help/errors.
    static void printUsage(CmdParser& parser);

    // Print a default configuration as a starting point.
    void printDefaultConfig();

    // Read a line and strip trailing return characters.
    std::string readLine();

    // Formulate the default cache directory path.
    std::string cacheDirectory();

    // Construct and print the XMLTV guide data.
    void printGuideData(std::ostream& out);

    // Initialise the configuration.
    void initConfiguration();

    // Fetch the list of channels and valid days.
    void fetchDatalist(XmlDoc& datalist);

    // Find a channel with the specified id.
    bool findChannel(XmlNav& nav, const std::string& id);

    // Fill in the channel with settings from the config.
    bool populateSettings(XmlNav nav, Channel& chan);

    // Fill in the channel with data from the source channel.
    bool populateSource(XmlNav nav, Channel& chan, bool fillin = false);

    // Convert string to integer with strict error checking.
    bool validPositiveInt(const std::string& str, int& num);

    // Convert string to integer with range check.
    bool validInt(const std::string& str, int& num, int min, int max);

    // Get the date string YYYY-MM-DD at the offset from today.
    std::string getDate(int offset);

    // Display a warning message.
    void warnUser(const std::string& msg);

    // Display an error message and exit quickly.
    void bailOut(const std::string& msg);

    // Parsed command line switches and arguments.
    unsigned char m_quietFlag;
    const char *m_outputPath;
    int m_daysToFetch;
    int m_offsetFromToday;
    const char *m_configFilePath;
    unsigned char m_debugFlag;
    int m_forcedTimeZone;

    // Collaborations with other objects.
    CmdParser& m_cmdParser;
    Config& m_config;
    HttpFetch& m_httpFetch;
    std::vector<Channel> m_channels;
};

#endif
