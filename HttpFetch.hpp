//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================
#ifndef HTTPFETCH_HPP
#define HTTPFETCH_HPP

#include <string>
#include <vector>
#include <map>
#include <curl/curl.h>

//////////////////////////////////////////////////////////////////////////////
// HttpFetch - singleton that implements a caching HTTP client
//////////////////////////////////////////////////////////////////////////////
class HttpFetch
{
    HttpFetch();
public:
    ~HttpFetch();

    // Return an instance of the HttpFetch singleton.
    static HttpFetch &instance();

    // Set the quiet flag.
    inline void setQuietFlag(bool quiet) { m_quietFlag = quiet; }

    // Set the path to the cache directory.
    inline void setCachePath(const std::string& cachePath) { m_cachePath = cachePath; }

    // Set the username for authenticated HTTP requests.
    inline void setUsername(const std::string& username) { m_username = username; }

    // Set the password for authenticated HTTP requests.
    inline void setPassword(const std::string& password) { m_password = password; }

    // Fetch a file from a HTTP server and save it in the cache.
    bool fetchFile(const std::vector<std::string>& base, const std::string& uri);

    // Get the error message associated with the last failed fetch.
    std::string getErrorMessage() const;

    // Read the entire contents of a file from the cache.
    bool getFileContents(const std::string& uri, std::string& contents);

    // Read the value of a header associated with the cached file.
    bool getHeader(const std::string& uri, const std::string& hdr, std::string& value);

    // Read all of the headers associated with the cached file.
    bool getHeaders(const std::string& uri, std::map<std::string,std::string>& headers);

    // Fetch a list of the files stored in the cache.
    void listCacheFiles(std::vector<std::string>& files);

    // Remove the specified cache file.
    bool removeCacheFile(const std::string& fileName);

private:
    // Initialise the cache and the curl library.
    bool initialise();

    // Delay for the specified time.
    void delay(int millisecs);

    // Select the next mirror site.
    void selectNextBase(const std::vector<std::string>& base);

    // Save data to a cache file.
    bool saveCacheFile(const std::string& fileName, const std::string& data);

    // Callback function used to save the fetched file to a string.
    static size_t saveToString(void *buffer, size_t size, size_t nmemb, void *userp);

    bool m_quietFlag;
    std::string m_cachePath;
    std::string m_username;
    std::string m_password;
    std::string m_base;
    CURL *m_easyHandle;
    CURLcode m_curlError;
    long m_responseCode;
};

#endif
