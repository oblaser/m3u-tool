/*
author          Oliver Blaser
date            12.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <stdexcept>
#include <string>
#include <vector>

#include "m3u.h"

#include <omw/string.h>


namespace
{
    inline bool isExtType(const std::string& line, const std::string& extBaseStr)
    {
        return (line.substr(0, extBaseStr.size()) == extBaseStr);
    }

    inline bool isExtType(const m3u::Entry& entry, const std::string& extBaseStr)
    {
        return (entry.ext().substr(0, extBaseStr.size()) == extBaseStr);
    }

    std::vector<std::string> readAllLines(const char* p, const char* const pEnd)
    {
        // TODO encoding check and conversion


/*
        // UTF BOM check
        if (txt.size() >= 4)
        {
            if (txt[0] == (char)(0x00) && txt[1] == (char)(0x00) &&
                txt[2] == (char)(0xFe) && txt[3] == (char)(0xFF))
            {
                printError(ewiFile, "encoding not supported: UTF-32 BE");
                return 1;
            }
            if (txt[0] == (char)(0xFF) && txt[1] == (char)(0xFe) &&
                txt[2] == (char)(0x00) && txt[3] == (char)(0x00))
            {
                printError(ewiFile, "encoding not supported: UTF-32 LE");
                return 1;
            }
        }

        if (txt.size() >= 2)
        {
            if (txt[0] == (char)(0xFe) && txt[1] == (char)(0xFF))
            {
                printError(ewiFile, "encoding not supported: UTF-16 BE");
                return 1;
            }
            if (txt[0] == (char)(0xFF) && txt[1] == (char)(0xFe))
            {
                printError(ewiFile, "encoding not supported: UTF-16 LE");
                return 1;
            }
        }

        bool isUTF8 = false;

        if (size >= 3)
        {
            // EF BB BF

            p += 3;

            isUTF8 = true;
        }

        if(file extension == "m3u8") isUTF8 = true;

        if (!isUTF8) iconv;
*/


        std::vector<std::string> lines;

        if (p < pEnd) lines.push_back("");

        while (p < pEnd)
        {
            const size_t nnlc = omw::peekNewLine(p, pEnd);
            
            if (nnlc == 0) lines.back().push_back(*p);
            else
            {
                lines.push_back("");
                if (nnlc > 1) ++p;
            }

            ++p;
        }

        return lines;
    }
}



const char* const m3u::ext_str = "#EXT";
const char* const m3u::extm3u_str = "#EXTM3U";
const char* const m3u::extenc_str = "#EXTENC:";
const char* const m3u::extinf_str = "#EXTINF:";
const char* const m3u::ext_x_media_str = "#EXT-X-MEDIA:";
const char* const m3u::ext_x_stream_inf_str = "#EXT-X-STREAM-INF:";

const char* const m3u::serializeEndOfLine = "\n";



void m3u::Entry::ExtParamValue::m_parse(const std::string& value)
{
    if (!value.empty())
    {
        if ((value[0] == '"') && (value.back() == '"'))
        {
            m_type = T_STRING;

            std::string tmp(value.begin() + 1, value.end() - 1);
            omw::replaceAll(tmp, "\"\"", '"');

            m_data = tmp;
        }
        else
        {
            if (omw::isInteger(value)) m_type = T_INTEGER;
            else m_type = T_SYMBOL;

            m_data = value;
        }
    }
}

bool m3u::Entry::ExtParamContainer::contains(const std::string& key) const
{
    bool r = false;
    
    for (size_type i = 0; i < size(); ++i)
    {
        if (at(i).key() == key)
        {
            r = true;
            break;
        }
    }

    return r;
}

const m3u::Entry::ExtParameter& m3u::Entry::ExtParamContainer::get(const std::string& key) const
{
    for (size_type i = 0; i < size(); ++i)
    {
        if (at(i).key() == key)
        {
            return at(i);
        }
    }

    throw std::out_of_range("no \"" + key + "\" parameter");
}

bool m3u::Entry::extIs(const std::string& extBaseStr) const
{
    return ::isExtType(*this, extBaseStr);
}

std::string m3u::Entry::serialize(const char* endOfLine) const
{
    std::string r = "";

    if (!m_ext.empty())
    {
        r += m_ext;

        if (!m_data.empty()) r += endOfLine;
    }

    if (!m_data.empty()) r += m_data;

    return r;
}

void m3u::Entry::m_parseExtData()
{
    const size_t colonPos = m_ext.find(':');

    if (colonPos != std::string::npos)
    {
        const char* p = m_ext.c_str() + colonPos + 1;
        const char* const pEnd = m_ext.c_str() + m_ext.length();

        m_extParam.clear();

        std::string key;
        std::string val;

        while (p < pEnd)
        {
            if (*p == '=')
            {
                ++p;
                if (p < pEnd)
                {
                    bool ignoreComma = false;

                    // proper quote interpretation is done in m3u::Entry::ExtParamValue ctor

                    while ((p < pEnd) &&
                           ((*p != ',') || ignoreComma))
                    {
                        if (*p == '"') ignoreComma = !ignoreComma;

                        val += *p;
                        ++p;
                    }
                }
            }
            else if (*p == ',')
            {
                if (!key.empty() && val.empty()) m_extParam.push_back(ExtParameter(std::string(), key));
                else if (!(key.empty() && val.empty())) m_extParam.push_back(ExtParameter(key, val));

                key.clear();
                val.clear();
                ++p;
            }
            else
            {
                key += *p;
                ++p;
            }
        }

        if (!key.empty() && val.empty()) m_extParam.push_back(ExtParameter("", key));
        else if (!(key.empty() && val.empty())) m_extParam.push_back(ExtParameter(key, val));
    }
}



std::string m3u::M3U::serialize(const char* endOfLine) const
{
    std::string r = "";

    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        r += m_entries[i].serialize(endOfLine) + endOfLine;
    }

    return r;
}

void m3u::M3U::m_parse(const char* p, const char* pEnd)
{
    const auto lines = ::readAllLines(p, pEnd);

    m_entries.clear();

    if (lines.size() > 0)
    {
        if (lines[0] == m3u::extm3u_str)
        {
            m_entries.push_back(m3u::Entry("", lines[0]));

            for (size_t i = 1; i < lines.size(); ++i)
            {
                if (!lines[i].empty())
                {
                    // is entry with extension
                    if (::isExtType(lines[i], m3u::extinf_str) ||
                        ::isExtType(lines[i], m3u::ext_x_stream_inf_str))
                    {
                        if (i < (lines.size() - 1))
                        {
                            m_entries.push_back(m3u::Entry(lines[i + 1], lines[i]));
                            ++i;
                        }
                        else m_entries.push_back(m3u::Entry("", lines[i]));
                    }
                    // is only extension
                    else if (::isExtType(lines[i], m3u::ext_str)) m_entries.push_back(m3u::Entry("", lines[i]));
                    // is entry
                    else m_entries.push_back(m3u::Entry(lines[i]));
                }
            }
        }
        else
        {
            for (size_t i = 0; i < lines.size(); ++i)
            {
                if (!lines[i].empty()) m_entries.push_back(m3u::Entry(lines[i]));
            }
        }

        if (m_entries.back().isEmpty()) m_entries.pop_back();
    }
}

void m3u::HLS::Subtitles::m_parse()
{
    for (const auto& param : m_extParam)
    {
        if (!param.value().empty())
        {
            if (param.key() == "LANGUAGE") m_language = param.value();
            else if (param.key() == "FORCED") m_forced = (param.value() == "YES");
            else if (param.key() == "URI") m_uri = param.value();
        }
    }
}

m3u::Entry::ExtParameter m3u::HLS::Stream::resolutionExtParam() const
{
    m3u::Entry::ExtParameter r;

    try
    {
        r = extParam().get("RESOLUTION");
    }
    catch (...)
    {
        r = m3u::Entry::ExtParameter("RESOLUTION", "-1x-1");
    }

    return r;
}

void m3u::HLS::Stream::m_parse()
{
    for (const auto& param : m_extParam)
    {
        if (!param.value().empty())
        {
            if (param.key() == "RESOLUTION")
            {
                const auto tokens = omw::split(param.value(), 'x');

                if ((tokens.size() == 2) && omw::isUInteger(tokens[0]) && omw::isUInteger(tokens[1]))
                {
                    m_resolutionHeight = std::stoi(tokens[1]);
                }
            }
            //else if (param.key() == "") ;
        }
    }
}

void m3u::HLS::m_parse()
{
    for (size_t i = 0; i < m_entries.size(); ++i)
    {
        const auto& e = m_entries[i];

        if (e.isExtension())
        {
            if (::isExtType(e, m3u::ext_x_media_str) && e.extParam().contains("TYPE"))
            {
                const auto& type = e.extParam().get("TYPE").value().data();

                if (type == "AUDIO") m_audioStreams.push_back(e);
                else if (type == "SUBTITLES") m_subtitles.push_back(e);
                else m_otherEntries.push_back(e);
            }
            else m_otherEntries.push_back(e);
        }
        else if (::isExtType(e, m3u::ext_x_stream_inf_str))
        {
            m_streams.push_back(e);
        }
        else m_otherEntries.push_back(e);
    }

    // do not clear entries, needed by serialize
}
