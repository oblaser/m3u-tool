/*
author          Oliver Blaser
date            16.08.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_MIDDLEWARE_M3U_H
#define IG_MIDDLEWARE_M3U_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>


namespace m3u
{
    // https://en.wikipedia.org/wiki/M3U

    class Entry
    {
    public:
        Entry() = delete;
        Entry(const std::string& path, const std::string& ext = std::string()) : m_ext(ext), m_path(path) {}
        virtual ~Entry() {}

        const std::string& path() const { return m_path; }

        bool isEmpty() const { return m_ext.empty() && m_path.empty(); }
        bool isExt() const { return !m_ext.empty() && m_path.empty(); }
        bool isRegular() const { return m_ext.empty() && !m_path.empty(); }
        bool hasExt() const { return !m_ext.empty(); }

        void setPath(const std::string& path) { m_path = path; }

        std::string serialize(const char* endOfLine = "\n") const;

    private:
        std::string m_ext;
        std::string m_path;
    };

    class M3U
    {
    public:
        M3U() noexcept : m_entries() {}
        explicit M3U(const std::string& file) : m_entries() { m_parseFile(file); }
        virtual ~M3U() {}

        const std::vector<m3u::Entry>& entries() const { return m_entries; }

        bool isEmpty() const { return m_entries.empty(); }
        bool isExt() const {}

        std::string serialize(const char* endOfLine = "\n") const;

    private:
        std::vector<m3u::Entry> m_entries;

        void m_parseFile(const std::string& file);
    };
}


#endif // IG_MIDDLEWARE_M3U_H
