/*
author          Oliver Blaser
date            27.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <cstdint>
#include <stdexcept>
#include <string>

#include "curl-helper.h"

// VS Properties:
//      - C/C++ > General > Additional Include Dirs: `$(sdk)/curl-7.81.0/x86/include`
//      - Linker > Input > Additional Dependencies: `Normaliz.lib;Ws2_32.lib;Wldap32.lib;Crypt32.lib;advapi32.lib;$(sdk)/curl-7.81.0/x86/lib/libcurl_a.lib;`
#define CURL_NO_OLDIES
#define CURL_STATICLIB
#include <curl/curl.h>



namespace
{
    using curl_data_t = std::string;

    size_t dataCallback(char* p, size_t size, size_t nmemb, void* pClientData)
    {
        size_t effSize = size * nmemb;
        for (size_t i = 0; i < effSize; ++i) ((curl_data_t*)pClientData)->push_back(*(p + i));
        return effSize;
    }

    struct ProgressData
    {
        ProgressData() = delete;
        ProgressData(const util::Curl* pCurl) : curl(pCurl) {}

        const util::Curl* const curl;
    };

    int progressCallback(void* pClientData, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        // https://curl.se/libcurl/c/progressfunc.html
        // https://curl.se/libcurl/c/CURLOPT_XFERINFOFUNCTION.html
        // https://curl.se/libcurl/c/CURLOPT_NOPROGRESS.html

        const ProgressData* const p = (const ProgressData*)pClientData;

        int r = CURL_PROGRESSFUNC_CONTINUE;

        if (p->curl->isAborted()) r = -1;

        return r;
    }
}



util::HttpGetResponse::HttpGetResponse()
    : m_curlCode(-1), m_httpCode(-1), m_data()
{ }

util::HttpGetResponse::HttpGetResponse(const std::string& data)
    : m_curlCode(-1), m_httpCode(-1), m_data(data)
{ }

util::HttpGetResponse::HttpGetResponse(int curlCode, int httpCode, const std::string& data)
    : m_curlCode(curlCode), m_httpCode(httpCode), m_data(data)
{ }

bool util::HttpGetResponse::good() const
{
    return ((m_curlCode == CURLE_OK) && (m_httpCode == 200));
}

bool util::HttpGetResponse::aborted() const
{
    return (m_curlCode == CURLE_ABORTED_BY_CALLBACK);
}

std::string util::HttpGetResponse::toString() const
{
    std::string str = std::to_string(m_curlCode);
    if (m_curlCode != CURLE_OK) str += " " + std::string(curl_easy_strerror((CURLcode)m_curlCode));
    str += " - " + std::to_string(m_httpCode) + " - " + m_data;
    return str;
}



const char* const util::Curl::defaultUserAgent = "libcurl";
size_t util::Curl::s_nInstances = 0;

util::Curl::Curl()
    : m_initDone(false), m_mtx(), m_abortState(false)
{
    if (s_nInstances < 1)
    {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK)
        {
            m_initDone = true;
            ++s_nInstances;
        }
    }
    else throw std::runtime_error("can't create more than one instance of curl");
}

util::Curl::~Curl()
{
    if (m_initDone)
    {
        curl_global_cleanup();
        --s_nInstances;
    }
    m_initDone = false;
}

util::HttpGetResponse util::Curl::httpGET(const std::string& reqStr, long timeoutConn, long timeout, const std::string& userAgent)
{
    util::HttpGetResponse r(-1, -1, "curl not initialized");

    if (m_initDone)
    {
        r = util::HttpGetResponse(-1, -1, "curl_easy_init() failed");

        CURL* curl = curl_easy_init();

        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, reqStr.c_str());
            curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());

            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeoutConn);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

            ::curl_data_t resBody;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resBody);

            const ProgressData progData(this);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1l); // XFERINFOFUNCTION won't be called, so the request can't be aborted (not needed in this project)
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
            curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progData);

            m_abort(false);

            CURLcode res;
            res = curl_easy_perform(curl);

            long resCode;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resCode);

            r = util::HttpGetResponse(res, resCode, resBody);

            curl_easy_cleanup(curl);
        }
    }

    return r;
}

bool util::Curl::isAborted() const
{
    std::lock_guard<std::mutex> lg(m_mtx);
    return m_abortState;
}

void util::Curl::abort()
{
    m_abort(true);
}

void util::Curl::m_abort(bool state)
{
    std::lock_guard<std::mutex> lg(m_mtx);
    m_abortState = state;
}
