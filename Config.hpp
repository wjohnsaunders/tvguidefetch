//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <vector>
#include <map>

#include "XmlDom.hpp"

//////////////////////////////////////////////////////////////////////////////
// Config - singleton used for accessing the grabber configuration
//////////////////////////////////////////////////////////////////////////////
class Config
{
public:
    // Class types.
    typedef std::map<std::string,std::string> MapOfConfigs;

    // Return an instance of the Config singleton.
    static Config& instance();

    // Set the path to the configuration file.
    bool setConfigPath(const std::string& path, bool dontLoad = false);

    // Fetch the value of a specified configuration item.
    bool getItem(const std::string& item, std::string& value);

    // Set the value of a specified configuration item.
    bool setItem(const std::string& item, const std::string& value);

    // Fetch the value of a list of items.
    bool getItemList(const std::string& parent, const std::string& item, std::vector<std::string>& values);

    // Return an XML navigation object for browsing the config file.
    XmlNav getConfigNav();

    // Register a default configuration.
    void registerDefaultConfig(const std::string& name, const std::string& config);

    // Load a default configuration.
    bool loadDefaultConfig(const std::string& listingsType);

    // Load the configuration from a file if not already loaded.
    bool loadConfig();

    // Write the configuration to the config path.
    bool writeConfig();

private:
    // Singleton pattern _must_ make these methods private.
    Config();
    ~Config();
    Config(const Config&);
    Config& operator=(const Config&);

    std::string m_configPath;
    XmlDoc m_configDoc;
    MapOfConfigs m_defaultConfigs;
};

#endif
