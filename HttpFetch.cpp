//============================================================================
// Name        : TvGuideFetch
// Author      : John Saunders
// Copyright   : Copyright (c) 2009 John Saunders
// Description : Fetch Australian XMLTV guide data from oztivo
//============================================================================

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <zlib.h>
#include <errno.h>

#include "EasyDate.hpp"
#include "HttpFetch.hpp"
#include "Version.hpp"

// CURLOPT_USERNAME and CURLOPT_PASSWORD came with version 7.19.1
#if LIBCURL_VERSION_MAJOR > 7
# define HAVE_USERNAME_PASSWORD
#else
# if LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR > 19
#  define HAVE_USERNAME_PASSWORD
# else
#  if LIBCURL_VERSION_MAJOR == 7 && LIBCURL_VERSION_MINOR == 19 && LIBCURL_VERSION_PATCH >= 1
#   define HAVE_USERNAME_PASSWORD
#  endif
# endif
#endif

//////////////////////////////////////////////////////////////////////////////
// HttpFetch - singleton that implements a caching HTTP client
//////////////////////////////////////////////////////////////////////////////
HttpFetch::HttpFetch()
:   m_quietFlag(false),
    m_cachePath("tvguide_cache"),
    m_easyHandle(0),
    m_curlError(CURLE_OK),
    m_responseCode(0)
{
    // Initialise the curl library.
    curl_global_init(CURL_GLOBAL_ALL);

    // Seed the random number generator with the current time stamp.
    srand(static_cast<unsigned int>(time(0)));
}

HttpFetch::~HttpFetch()
{
    curl_global_cleanup();
}

// Return an instance of the HttpFetch singleton.
HttpFetch &HttpFetch::instance()
{
    static HttpFetch s_instance;
    return s_instance;
}

// Initialise the cache and the curl library.
bool HttpFetch::initialise()
{
    if (!mkCacheDir())
    {
        return false;
    }

    // Initialise the CURL handle for doing fetches.
    if (m_easyHandle == 0)
    {
        if ((m_easyHandle = curl_easy_init()) == 0)
        {
            return false;
        }
    }

    return true;
}

// Create directory used for the cache.
bool HttpFetch::mkCacheDir()
{
    struct stat info;

    if ((stat(m_cachePath.c_str(), &info) == 0) &&
        ((info.st_mode & S_IFDIR) != 0))
    {
        return true;
    }
    else
    {
        // Create the cache directory if it doesn't already exist.
#ifdef WIN32
        int result = _mkdir(m_cachePath.c_str());
#else
        int result = mkdir(m_cachePath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
        if (result == 0)
        {
            return true;
        }
        else
        {
            std::cerr << strerror(errno) << ": Creating directory '" << m_cachePath.c_str() << "' failed!" << std::endl;
            return false;
        }
    }
}

// Delay for the specified time.
void HttpFetch::delay(int millisecs)
{
#ifdef WIN32
        Sleep(millisecs);
#else
        struct timespec req = { 0, millisecs * 1000000 };
        nanosleep(&req, &req);
#endif
}

// Select the next mirror site.
void HttpFetch::selectNextBase(const std::vector<std::string>& base)
{
    if (base.size() > 1)
    {
#if __cplusplus>=201103L
        std::uniform_int_distribution<size_t> distribution(0, base.size() - 1);
        size_t newMirror = distribution(m_generator);
#else
        size_t newMirror = rand() % base.size();
#endif

        if (m_base.empty())
        {
            // Select the first mirror randomly from the provided list.
            m_base = base[newMirror];
        }
        else
        {
            // Select the next mirror randomly, but advance to the next one
            // if by chance we selected the same mirror as last time.
            if (base[newMirror] == m_base)
            {
                newMirror = (newMirror + 1) % base.size();
            }
            m_base = base[newMirror];
        }

        // Sleep to maintain a 1 second delay between file
        // fetches from the same server. Only need to sleep
        // 0.5 seconds with more than 1 mirror available.
        delay(500);
    }
    else if (base.size() == 1)
    {
        // Select the first and only mirror.
        m_base = base[0];

        // Sleep to maintain a 1 second delay between file
        // fetches from the same server.
        delay(1000);
    }
    else
    {
        std::cerr << "Error: No tvguidefetch servers available!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

// Fetch a file from a HTTP server and save it in the cache.
bool HttpFetch::fetchFile(const std::vector<std::string>& base, const std::string& uri)
{
    // Ensure the cache and CURL easy handle are initialised.
    if (!initialise())
    {
        return false;
    }

    // Create the full URL from the selected base.
    selectNextBase(base);
    std::string url = m_base + uri;

    // Create the headers used to prevent fetching of not modified files.
    struct curl_slist *requestHeaders = 0;
    requestHeaders = curl_slist_append(requestHeaders, "Accept-Encoding: gzip, deflate, x-gzip, x-deflate, identity");
    std::map<std::string,std::string> fileHeaders;
    if (getHeaders(uri, fileHeaders))
    {
        std::map<std::string,std::string>::iterator iter;
        if ((iter = fileHeaders.find("Date")) != fileHeaders.end())
        {
            EasyDate lastModified;
            if (lastModified.fromRfc822(iter->second))
            {
                EasyDate currentTime;
                lastModified.setHour(lastModified.getHour() + 1);
                if (lastModified > currentTime)
                {
                    // Recently cached, don't fetch.
                    if (!m_quietFlag)
                    {
                        std::cout << url << " (Recently cached)" << std::endl;
                    }
                    return true;
                }
            }
        }
        if ((iter = fileHeaders.find("Last-Modified")) != fileHeaders.end())
        {
            std::string ifModifiedSince("If-Modified-Since: ");
            ifModifiedSince.append(iter->second);
            requestHeaders = curl_slist_append(requestHeaders, ifModifiedSince.c_str());
        }
        if ((iter = fileHeaders.find("ETag")) != fileHeaders.end())
        {
            std::string ifNoneMatch("If-None-Match: ");
            ifNoneMatch.append(iter->second);
            requestHeaders = curl_slist_append(requestHeaders, ifNoneMatch.c_str());
        }
    }
    requestHeaders = curl_slist_append(requestHeaders, "Cache-Control: no-cache");
    requestHeaders = curl_slist_append(requestHeaders, "Pragma: no-cache");

    // Configure CURL to fetch the URL contents and header into strings.
    curl_easy_setopt(m_easyHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_easyHandle, CURLOPT_USERAGENT, USERAGENT);
    curl_easy_setopt(m_easyHandle, CURLOPT_HTTPHEADER, requestHeaders);

#ifdef HAVE_USERNAME_PASSWORD
    curl_easy_setopt(m_easyHandle, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(m_easyHandle, CURLOPT_PASSWORD, m_password.c_str());
#else
    std::string userpwd = m_username + ":" + m_password;
    curl_easy_setopt(m_easyHandle, CURLOPT_USERPWD, userpwd.c_str());
#endif
    curl_easy_setopt(m_easyHandle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    curl_easy_setopt(m_easyHandle, CURLOPT_FAILONERROR, 1);
    std::string dataContents;
    curl_easy_setopt(m_easyHandle, CURLOPT_WRITEFUNCTION, HttpFetch::saveToString);
    curl_easy_setopt(m_easyHandle, CURLOPT_WRITEDATA, &dataContents);
    std::string headerContents;
    curl_easy_setopt(m_easyHandle, CURLOPT_HEADERFUNCTION, HttpFetch::saveToString);
    curl_easy_setopt(m_easyHandle, CURLOPT_HEADERDATA, &headerContents);

    // Perform the HTTP transfer
    //curl_easy_setopt(m_easyHandle, CURLOPT_VERBOSE, 1);
    m_curlError = curl_easy_perform(m_easyHandle);
    curl_slist_free_all(requestHeaders);

    // Check for error to remove cached files.
    switch (m_curlError)
    {
    case CURLE_OK:
        // CURL returns this for HTTP status codes 2xx and 3xx. We treat 2xx as a
        // successful fetch, 304 as not modified and also a success, but other 3xx
        // codes as a failure.
        if (curl_easy_getinfo(m_easyHandle, CURLINFO_RESPONSE_CODE, &m_responseCode) == CURLE_OK)
        {
            if (m_responseCode < 300)
            {
                // Fetched, save the data and headers.
                if (!m_quietFlag)
                {
                    std::cout << url << " (Fetched)" << std::endl;
                }
                break;
            }
            else if (m_responseCode == 304)
            {
                // Not modified, don't save the data or headers.
                if (!m_quietFlag)
                {
                    std::cout << url << " (Not modified)" << std::endl;
                }
                return true;
            }
        }
        return false;

    case CURLE_HTTP_RETURNED_ERROR:
        // CURL returns this for HTTP status codes >= 400, save the code for later use.
        if (curl_easy_getinfo(m_easyHandle, CURLINFO_RESPONSE_CODE, &m_responseCode) != CURLE_OK)
        {
            m_responseCode = 0;
        }
        return false;

    default:
        return false;
    }

    // Create the cache file for the data and header.
    if (saveCacheFile(uri, dataContents) &&
        saveCacheFile(uri + ".header", headerContents))
    {
        return true;
    }

    return false;
}

// Save data to a cache file.
bool HttpFetch::saveCacheFile(const std::string& fileName, const std::string& data)
{
    // Create the full path name.
    std::string dataPath = m_cachePath + "/" + fileName;

    // Save the data to the file.
    std::ofstream out;
    out.open(dataPath.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    if (out.is_open())
    {
        out << data;
        if (out.bad()) return false;
        out.close();

        return true;
    }

    return false;
}

// Callback function used to save the fetched file to a string.
size_t HttpFetch::saveToString(void *buffer, size_t size, size_t nmemb, void *userp)
{
    std::string *s = reinterpret_cast<std::string *>(userp);
    s->append(reinterpret_cast<char *>(buffer), size * nmemb);
    return nmemb;
}

// Get the error message associated with the last failed fetch.
std::string HttpFetch::getErrorMessage() const
{
    std::stringstream msg;

    switch (m_curlError)
    {
    case CURLE_OK:
        return std::string("Ok");

    case CURLE_COULDNT_RESOLVE_HOST:
        return std::string("Couldn't resolve the hostname");

    case CURLE_COULDNT_CONNECT:
        return std::string("Couldn't connect to server");

    case CURLE_HTTP_RETURNED_ERROR:
        msg << "HTTP server returned an error";
        if (m_responseCode >= 400)
        {
            msg << " code " << m_responseCode;
        }
        return msg.str();

    default:
        msg << "Error code " << m_curlError << " occurred!";
        return msg.str();
    }
}

// Read the entire contents of a file from the cache.
bool HttpFetch::getFileContents(const std::string& uri, std::string &contents)
{
    std::string contentEncoding;
    bool compressedEncoding = false;
    z_stream zlibStrm;
    int zlibReturn = Z_STREAM_END;

    // Determine the content encoding so we can uncompress the contents.
    if (getHeader(uri, std::string("Content-Encoding"), contentEncoding))
    {
        if ((contentEncoding == "x-gzip") || (contentEncoding == "gzip") ||
            (contentEncoding == "x-deflate") || (contentEncoding == "deflate"))
        {
            compressedEncoding = true;
        }
    }

    // Initialise the zlib uncompress library.
    if (compressedEncoding)
    {
        zlibStrm.zalloc = Z_NULL;
        zlibStrm.zfree = Z_NULL;
        zlibStrm.opaque = Z_NULL;
        zlibStrm.avail_in = 0;
        zlibStrm.next_in = Z_NULL;
        // 32 is added for automatic selection of zlib or gzip
        if (inflateInit2(&zlibStrm, 15 + 32) != Z_OK)
            return false;
    }

    // Read the data from the cache file.
    std::string dataPath = m_cachePath + "/" + uri;
    std::ifstream in;
    in.open(dataPath.c_str(), std::ios_base::in | std::ios_base::binary);
    if (in.is_open())
    {
        contents.clear();
        char buffer[4096];
        while (in.good())
        {
            in.read(buffer, sizeof(buffer));
            if (in.bad()) break;

            // Uncompress the file on-the-fly.
            if (compressedEncoding)
            {
                zlibStrm.next_in = reinterpret_cast<Bytef *>(buffer);
                zlibStrm.avail_in = in.gcount();
                do {
                    char outbuf[4096];
                    zlibStrm.next_out = reinterpret_cast<Bytef *>(outbuf);
                    zlibStrm.avail_out = sizeof(outbuf);
                    int zlibReturn = inflate(&zlibStrm, Z_NO_FLUSH);
                    switch (zlibReturn)
                    {
                    case Z_NEED_DICT:
                        zlibReturn = Z_DATA_ERROR;     /* and fall through */
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                    case Z_STREAM_ERROR:
                        inflateEnd(&zlibStrm);
                        return false;
                    }
                    contents += std::string(outbuf, sizeof(outbuf) - zlibStrm.avail_out);
                } while (zlibStrm.avail_out == 0);
            }
            else
            {
                contents += std::string(buffer, in.gcount());
            }
        }
        in.close();
        if (compressedEncoding)
        {
            inflateEnd(&zlibStrm);
        }
        return (zlibReturn == Z_STREAM_END);
    }

    return false;
}

// Read the value of a header associated with the cached file.
bool HttpFetch::getHeader(const std::string& uri, const std::string& hdr, std::string &value)
{
    // Read the headers from the cache file.
    std::string headerPath = m_cachePath + "/" + uri + ".header";
    std::ifstream in;
    in.open(headerPath.c_str(), std::ios_base::in | std::ios_base::binary);
    if (in.is_open())
    {
        char buffer[4096];
        while (in.good())
        {
            in.getline(buffer, sizeof(buffer));
            if (in.bad()) break;

            std::string line(buffer);
            size_t pos = line.find('\r');
            if (pos != std::string::npos)
            {
                line = line.substr(0, pos);
            }
            pos = line.find(": ");
            if (pos != std::string::npos)
            {
                if (line.substr(0, pos) == hdr)
                {
                    value = line.substr(pos + 2);
                    break;
                }
            }
            else if (line.substr(0, 5) == "HTTP/")
            {
                if (line.substr(0, 4) == hdr)
                {
                    value = line.substr(5);
                    break;
                }
            }
        }
        in.close();

        return true;
    }

    return false;
}

// Read all of the headers associated with the cached file.
bool HttpFetch::getHeaders(const std::string& uri, std::map<std::string,std::string>& headers)
{
    // Read the headers from the cache file.
    std::string headerPath = m_cachePath + "/" + uri + ".header";
    std::ifstream in;
    in.open(headerPath.c_str(), std::ios_base::in | std::ios_base::binary);
    if (in.is_open())
    {
        headers.clear();

        char buffer[4096];
        while (in.good())
        {
            in.getline(buffer, sizeof(buffer));
            if (in.bad()) break;

            std::string line(buffer);
            size_t pos = line.find('\r');
            if (pos != std::string::npos)
            {
                line = line.substr(0, pos);
            }
            pos = line.find(": ");
            if (pos != std::string::npos)
            {
                headers[line.substr(0, pos)] = line.substr(pos + 2);
            }
            else if (line.substr(0, 5) == "HTTP/")
            {
                headers[line.substr(0, 4)] = line.substr(5);
            }
        }
        in.close();

        return true;
    }

    return false;
}

// Fetch a list of the files stored in the cache.
void HttpFetch::listCacheFiles(std::vector<std::string>& files)
{
    DIR *dir;

    // Populate the vector with the list of files, excluding the .header files.
    if ((dir = opendir(m_cachePath.c_str())) != 0)
    {
        struct dirent *dirent;
        while ((dirent = readdir(dir)) != 0)
        {
            std::string fileName(dirent->d_name);
            size_t pos = fileName.find(".header");
            if ((pos == std::string::npos) ||
                (pos != (fileName.size() - 7)))
            {
                files.push_back(fileName);
            }
        }
        closedir(dir);
    }

    // Sort the list, although probably not needed.
    std::sort(files.begin(), files.end());
}

// Remove the specified cache file.
bool HttpFetch::removeCacheFile(const std::string& fileName)
{
    if (!m_quietFlag)
    {
        std::cout << fileName << " (Removing cache file)" << std::endl;
    }
    int result1 = remove((m_cachePath + "/" + fileName).c_str());
    int result2 = remove((m_cachePath + "/" + fileName + ".header").c_str());
    return (result1 == 0) && (result2 == 0);
}
