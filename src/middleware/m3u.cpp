/*
author          Oliver Blaser
date            19.08.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <string>
#include <string_view>
#include <vector>

#include "m3u.h"

#include <omw/io/file.h>
#include <omw/string.h>


namespace fs = std::filesystem;

namespace
{
    constexpr std::string_view extm3u_str = "#EXTM3U";
    constexpr std::string_view extinf_str = "#EXTINF:";



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
                if (lines[i].substr(0, ::extinf_str.size()) == extinf_str)
                {
                    if (i < (lines.size() - 1))
                    {
                        m_entries.push_back(m3u::Entry(lines[i + 1], lines[i]));
                        ++i;
                    }
                    else m_entries.push_back(m3u::Entry("", lines[i]));
                }
                else m_entries.push_back(lines[i]);
            }
        }
        else
        {
            for (size_t i = 0; i < lines.size(); ++i)
            {
                m_entries.push_back(lines[i]);
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
