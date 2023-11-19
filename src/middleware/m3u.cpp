/*
author          Oliver Blaser
date            19.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <string>
#include <vector>

#include "m3u.h"

#include <omw/io/file.h>
#include <omw/string.h>


namespace fs = std::filesystem;

namespace
{
    const char* const ext_str = "#EXT";
    const char* const extm3u_str = "#EXTM3U";
    const char* const extinf_str = "#EXTINF:";
    const char* const ext_x_stream_inf_str = "#EXT-X-STREAM-INF:";

    inline bool isExtType(const std::string& line, const std::string& extTypeStr)
    {
        return (line.substr(0, extTypeStr.size()) == extTypeStr);
    }



    std::vector<std::string> readAllLines(const std::string& file)
    {
        const auto fileIf = omw::io::TxtFileInterface(file);
        fileIf.openRead();
        const size_t fileSize = fileIf.size();

        std::string ___txt(fileSize, ' ');
        const std::string& txt = ___txt;

        fileIf.read(___txt.data(), ___txt.size());

        const char* p = txt.data();
        const char* const pEnd = txt.data() + txt.size();








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



void m3u::Entry::ExtParamValue::m_parse(const std::string& value)
{
    if (!value.empty())
    {
        if ((value[0] == '"') && (value.back() == '"'))
        {
            m_type = T_STRING;

            std::string tmp(value.begin() + 1, value.end() - 1);
            omw::replaceAll(tmp, "\"\"", '"');

            m_value = tmp;
        }
        else
        {
            if (omw::isInteger(value)) m_type = T_INTEGER;
            else m_type = T_SYMBOL;

            m_value = value;
        }
    }
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

        m_extData.clear();

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
                if (!key.empty() && val.empty()) m_extData.push_back(ExtParameter(std::string(), key));
                else if (!(key.empty() && val.empty())) m_extData.push_back(ExtParameter(key, val));

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

        if (!key.empty() && val.empty()) m_extData.push_back(ExtParameter("", key));
        else if (!(key.empty() && val.empty())) m_extData.push_back(ExtParameter(key, val));
    }
}



void m3u::M3U::m_parseFile(const std::string& file)
{
    const auto lines = ::readAllLines(file);

    m_entries.clear();

    if (lines.size() > 0)
    {
        if (lines[0] == ::extm3u_str)
        {
            m_entries.push_back(m3u::Entry("", lines[0]));

            for (size_t i = 1; i < lines.size(); ++i)
            {
                if (!lines[i].empty())
                {
                    // is entry with extension
                    if (::isExtType(lines[i], ::extinf_str) ||
                        ::isExtType(lines[i], ::ext_x_stream_inf_str))
                    {
                        if (i < (lines.size() - 1))
                        {
                            m_entries.push_back(m3u::Entry(lines[i + 1], lines[i]));
                            ++i;
                        }
                        else m_entries.push_back(m3u::Entry("", lines[i]));
                    }
                    // is only extension
                    else if (::isExtType(lines[i], ::ext_str)) m_entries.push_back(m3u::Entry("", lines[i]));
                    // is entry
                    else m_entries.push_back(lines[i]);
                }
            }
        }
        else
        {
            for (size_t i = 0; i < lines.size(); ++i)
            {
                if (!lines[i].empty()) m_entries.push_back(lines[i]);
            }
        }

        if (m_entries.back().isEmpty()) m_entries.pop_back();
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
