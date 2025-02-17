/*
author          Oliver Blaser
date            23.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#ifndef IG_MIDDLEWARE_UTIL_H
#define IG_MIDDLEWARE_UTIL_H

#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>

#include "omw/string.h"


namespace util {

class FileCounter_Base
{
public:
    using counter_type = size_t;

public:
    FileCounter_Base()
        : m_total(0), m_value(0)
    {}

    virtual ~FileCounter_Base() {}

    void addTotal(counter_type value = 1) { m_total += value; }

    const counter_type& total() const { return m_total; }

protected:
    void addValue(counter_type value = 1) { m_value += value; }
    const counter_type& value() const { return m_value; }

private:
    counter_type m_total;
    counter_type m_value;
};

class CopyFileCounter : public FileCounter_Base
{
public:
    CopyFileCounter()
        : FileCounter_Base()
    {}

    virtual ~CopyFileCounter() {}

    void addCopied(counter_type value = 1) { addValue(value); }

    const counter_type& copied() const { return value(); }
};

class ExistsFileCounter : public FileCounter_Base
{
public:
    ExistsFileCounter()
        : FileCounter_Base()
    {}

    virtual ~ExistsFileCounter() {}

    void addExists(counter_type value = 1) { addValue(value); }

    const counter_type& exists() const { return value(); }
};

class ResultCounter
{
public:
    using counter_type = size_t;

public:
    ResultCounter()
        : m_e(0), m_w(0)
    {}

    ResultCounter(counter_type e, counter_type w = 0)
        : m_e(e), m_w(w)
    {}

    virtual ~ResultCounter() {}

    counter_type errors() const { return m_e; };
    counter_type warnings() const { return m_w; };

    void incErrors() { ++m_e; }
    void incWarnings() { ++m_w; }

private:
    counter_type m_e;
    counter_type m_w;
};

class Uri
{
public:
    Uri()
        : m_validity(false), m_scheme(), m_authority(), m_path(), m_query(), m_fragment()
    {}

    explicit Uri(const std::string& uri)
        : m_validity(false), m_scheme(), m_authority(), m_path(), m_query(), m_fragment()
    {
        set(uri);
    }

    virtual ~Uri() {}

    void set(const std::string& uri);
    void clear();

    const std::string& scheme() const { return m_scheme; }
    const std::string& authority() const { return m_authority; }
    const std::string& path() const { return m_path; }
    const std::string& query() const { return m_query; }
    const std::string& fragment() const { return m_fragment; }

    void setScheme(const std::string& scheme) { m_scheme = scheme; }
    void setAuthority(const std::string& authority) { m_authority = authority; }
    void setPath(const std::string& path) { m_path = path; }
    void setQuery(const std::string& query) { m_query = query; }
    void setFragment(const std::string& fragment) { m_fragment = fragment; }

    std::string string() const;

    bool isValid() const { return m_validity; }


    bool isUrl() const;

private:
    bool m_validity;
    std::string m_scheme;
    std::string m_authority;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
};

std::string getDateTimeStr(time_t t = std::time(nullptr), const char* strftimeFormat = "%FT%T%z");

omw::string getDirName(const std::filesystem::path& dir);

std::string readFile(const std::filesystem::path& file);
void writeFile(const std::filesystem::path& file, const std::string& text);
} // namespace util


#include <omw/omw.h>
namespace omw_ {
#if OMW_VERSION_ID <= OMW_VERSION_ID_0_2_1_BETA

namespace cli {
    int choice(const std::string& q, int def = 0, char first = 'y', char second = 'n');
}

omw::string to_string(uint64_t val, int base, const char* digits);

std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);
std::string& upper(std::string& str); // do not implement!
#endif

} // namespace omw_


#endif // IG_MIDDLEWARE_UTIL_H
