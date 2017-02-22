//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================

#include <iostream>
#include <fstream>

#include "Config.hpp"

//////////////////////////////////////////////////////////////////////////////
// Config - singleton used for accessing the grabber configuration
//////////////////////////////////////////////////////////////////////////////
Config::Config()
:   m_configPath("tvguide_config.xml")
{
}

Config::~Config()
{
}

// Return an instance of the Config singleton.
Config& Config::instance()
{
    static Config s_instance;
    return s_instance;
}

// Set the path to the configuration file.
bool Config::setConfigPath(const std::string& path, bool dontLoad)
{
    m_configPath = path;

    return dontLoad || loadConfig();
}

// Fetch the value of a specified configuration item.
bool Config::getItem(const std::string& item, std::string& value)
{
    if (!loadConfig())
    {
        return false;
    }

    XmlNav nav(m_configDoc);

    if (nav.gotoNode(item))
    {
        value = nav.getAllChars();
        return true;
    }

    return false;
}

// Set the value of a specified configuration item.
bool Config::setItem(const std::string& item, const std::string& value)
{
    if (!loadConfig())
    {
        return false;
    }

    XmlNav nav(m_configDoc);

    if (nav.gotoNode(item))
    {
        nav.setAllChars(value.c_str());
        return true;
    }

    return false;
}

// Fetch the value of a list of items.
bool Config::getItemList(const std::string& parent, const std::string& item, std::vector<std::string>& values)
{
    if (!loadConfig())
    {
        return false;
    }

    XmlNav nav(m_configDoc);

    if (nav.gotoNode(parent))
    {
        std::string itemName(item);
        values.clear();
        bool nodeSuccess = nav.gotoFirstChildNode();
        while (nodeSuccess)
        {
            if (itemName == nav.getNodeName())
            {
                values.push_back(nav.getAllChars());
            }
            nodeSuccess = nav.gotoNextSiblingNode();
        }

        return true;
    }

    return false;
}

// Return an XML navigation object for browsing the config file.
XmlNav Config::getConfigNav()
{
    return XmlNav(m_configDoc);
}

// Register a default configuration.
void Config::registerDefaultConfig(const std::string& name, const std::string& config)
{
    m_defaultConfigs[name] = config;
}

// Load a default configuration.
bool Config::loadDefaultConfig(const std::string& listingsType)
{
    MapOfConfigs::const_iterator iter = m_defaultConfigs.find(listingsType);
    if (iter != m_defaultConfigs.end())
    {
        return m_configDoc.parse((*iter).second, true);
    }

    return false;
}

// Load the configuration from a file if not already loaded.
bool Config::loadConfig()
{
    if (!m_configDoc.isLoaded())
    {
        std::ifstream in;
        in.open(m_configPath.c_str(), std::ios_base::in);
        if (in.is_open())
        {
            bool result = m_configDoc.parse(in, true);
            in.close();
            return result;
        }

        return false;
    }

    return true;
}

// Write the configuration to the config path.
bool Config::writeConfig()
{
    XmlNav nav(m_configDoc);

    std::ofstream out;
    out.open(m_configPath.c_str(), std::ios_base::out | std::ios_base::trunc);
    if (out.is_open())
    {
        out << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << std::endl
            << "<!-- TvGuideFetch configuration -->" << std::endl
            << nav << std::endl;
        bool result = !out.bad();
        out.close();
        return result;
    }

    return false;
}
