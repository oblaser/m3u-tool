/*
author          Oliver Blaser
date            19.08.2023
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
        Entry(const std::string& data, const std::string& ext = std::string()) : m_ext(ext), m_data(data) {}
        virtual ~Entry() {}

        const std::string& data() const { return m_data; }
        std::string& path() { return m_data; }
        const std::string& path() const { return m_data; }
        std::string& ext() { return m_ext; }
        const std::string& ext() const { return m_ext; }

        bool isEmpty() const { return m_ext.empty() && m_data.empty(); }
        bool isComment() const { return m_data.substr(0, 1) == "#"; }
        bool isExtension() const { return !m_ext.empty() && m_data.empty(); }
        bool isResource() const { return !m_data.empty() && !isComment(); } // is resource (with or without extension)
        bool hasExtension() const { return !m_ext.empty(); }
        bool isRegularRes() const { return isResource() && !hasExtension(); }

        void setData(const std::string& data) { m_data = data; }

        std::string serialize(const char* endOfLine = "\n") const;

    private:
        std::string m_ext;
        std::string m_data;
    };

    class M3U
    {
    public:
        M3U() noexcept : m_entries() {}
        explicit M3U(const std::string& file) : m_entries() { m_parseFile(file); }
        virtual ~M3U() {}

        std::vector<m3u::Entry>& entries() { return m_entries; }
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
