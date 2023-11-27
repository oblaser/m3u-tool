/*
author          Oliver Blaser
date            27.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_MIDDLEWARE_UTIL_H
#define IG_MIDDLEWARE_UTIL_H

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>

#include "omw/string.h"


namespace util
{
    class FileCounter
    {
    public:
        using counter_type = size_t;

    public:
        FileCounter() : m_total(0), m_copied(0) {}
        virtual ~FileCounter() {}

        FileCounter& add(counter_type total, counter_type copied) { m_total += total; m_copied += copied; return (*this); }
        FileCounter& add(const FileCounter& other) { return add(other.total(), other.copied()); }
        FileCounter& addTotal(counter_type value = 1) { m_total += value; return (*this); }
        FileCounter& addCopied(counter_type value = 1) { m_copied += value; return (*this); }

        const counter_type& total() const { return m_total; }
        const counter_type& copied() const { return m_copied; }

    private:
        counter_type m_total;
        counter_type m_copied;
    };

    class ResultCounter
    {
    public:
        using counter_type = size_t;

    public:
        ResultCounter() : m_e(0), m_w(0) {}
        ResultCounter(counter_type e, counter_type w = 0) : m_e(e), m_w(w) {}
        virtual ~ResultCounter() {}

        counter_type errors() const { return m_e; };
        counter_type warnings() const { return m_w; };

        void incErrors() { ++m_e; }
        void incWarnings() { ++m_w; }

    private:
        counter_type m_e;
        counter_type m_w;
    };

    std::string getDateTimeStr(time_t t = std::time(nullptr), const char* strftimeFormat = "%FT%T%z");

    void printFormattedText(const std::string& text);
    void printFormattedLine(const std::string& text);
    void printError(const std::string& text);
    void printInfo();
    void printInfo(const std::string& text);
    void printWarning(const std::string& text);
    void printTitle(const std::string& title);
    omw::string getDirName(const std::filesystem::path& dir);
}


#include <omw/omw.h>
namespace omw_
{
#if OMW_VERSION_ID <= OMW_VERSION_ID_0_2_1_ALPHA_2
    namespace cli { int choice(const std::string& q, int def = 0, char first = 'y', char second = 'n'); }
    omw::string to_string(uint64_t val, int base, const char* digits);
#endif
}


#endif // IG_MIDDLEWARE_UTIL_H
