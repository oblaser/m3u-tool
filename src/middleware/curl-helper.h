/*
author          Oliver Blaser
date            27.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_MIDDLEWARE_CURLHELPER_H
#define IG_MIDDLEWARE_CURLHELPER_H

#include <cstdint>
#include <mutex>
#include <string>

namespace util
{
    class HttpGetResponse
    {
    public:
        HttpGetResponse();
        HttpGetResponse(const std::string& data);
        HttpGetResponse(int curlCode, int httpCode, const std::string& data);
        virtual ~HttpGetResponse() {}

        int curlCode() const { return m_curlCode; }
        int httpCode() const { return m_httpCode; }
        const std::string& data() const { return m_data; }

        bool good() const;
        bool aborted() const;

        std::string toString() const;

    private:
        int m_curlCode;
        int m_httpCode;
        std::string m_data;
    };

    class Curl
    {
    public:
        static const char* const defaultUserAgent;

    public:
        Curl();
        virtual ~Curl();

        bool isInitDone() const { return m_initDone; }

        HttpGetResponse httpGET(const std::string& reqStr, long timeoutConn = 0, long timeout = 0, const std::string& userAgent = defaultUserAgent);

        bool isAborted() const;
        void abort();

    private:
        bool m_initDone;

        bool m_abortState;
        mutable std::mutex m_mtx;

        void m_abort(bool state);

    private:
        Curl(const Curl& other) = delete;
        Curl& operator=(const Curl&);

        static size_t s_nInstances;
    };
}

#endif // IG_MIDDLEWARE_CURLHELPER_H
