/*
author          Oliver Blaser
date            16.08.2023
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

        if (!m_path.empty()) r += endOfLine;
    }

    if (!m_path.empty()) r += m_path;

    return r;
}



void m3u::M3U::m_parseFile(const std::string& file)
{
    const auto lines = ::readAllLines(file);

    m_entries.clear();
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
