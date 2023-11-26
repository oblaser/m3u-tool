/*
author          Oliver Blaser
date            26.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_MIDDLEWARE_M3U_H
#define IG_MIDDLEWARE_M3U_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>


namespace m3u
{
    const char* const serializeEndOfLine = "\n";

    class Entry
    {
    public:
        class ExtParamValue
        {
        public:
            enum
            {
                T_UNKNOWN = 0,
                T_INTEGER,
                T_STRING,
                T_SYMBOL,
            };

        public:
            ExtParamValue() : m_type(T_UNKNOWN), m_data() {}
            ExtParamValue(const std::string& value) : m_type(T_UNKNOWN), m_data() { m_parse(value); }
            virtual ~ExtParamValue() {}

            int type() const { return m_type; }
            const std::string& data() const { return m_data; }

            bool empty() const { return m_data.empty(); }

            operator const std::string& () const { return m_data; }

        private:
            int m_type;
            std::string m_data;

            void m_parse(const std::string& value);
        };

        class ExtParameter : private std::pair<std::string, m3u::Entry::ExtParamValue>
        {
        public:
            ExtParameter() : std::pair<std::string, m3u::Entry::ExtParamValue>(), m_validity(false) {}
            ExtParameter(const std::string& key, const ExtParamValue& value) : std::pair<std::string, m3u::Entry::ExtParamValue>(key, value), m_validity(true) {}
            virtual ~ExtParameter() {}

            const std::string& key() const { return first; }
            const m3u::Entry::ExtParamValue& value() const { return second; }

            const bool isValid() const { return m_validity; }

        private:
            bool m_validity;
        };

        class ExtParamContainer : public std::vector<m3u::Entry::ExtParameter>
        {
        public:
            ExtParamContainer() : std::vector<m3u::Entry::ExtParameter>() {}
            virtual ~ExtParamContainer() {}

            bool contains(const std::string& key) const;
            const m3u::Entry::ExtParameter& get(const std::string& key) const;
        };

    public:
        Entry() = delete;
        Entry(const std::string& data, const std::string& ext = std::string()) : m_ext(ext), m_extParam(), m_data(data) { m_parseExtData(); }
        virtual ~Entry() {}

        const std::string& data() const { return m_data; }
        std::string& path() { return m_data; }
        const std::string& path() const { return m_data; }
        std::string& ext() { return m_ext; }
        const std::string& ext() const { return m_ext; }
        const ExtParamContainer& extParam() const { return m_extParam; }

        bool isEmpty() const { return m_ext.empty() && m_data.empty(); }
        bool isComment() const { return m_data.substr(0, 1) == "#"; }
        bool isExtension() const { return !m_ext.empty() && m_data.empty(); }
        bool isResource() const { return !m_data.empty() && !isComment(); } // is resource (with or without extension)
        bool hasExtension() const { return !m_ext.empty(); }
        bool isRegularRes() const { return isResource() && !hasExtension(); }

        void setData(const std::string& data) { m_data = data; }

        std::string serialize(const char* endOfLine = serializeEndOfLine) const;

    protected:
        std::string m_ext;
        m3u::Entry::ExtParamContainer m_extParam;
        std::string m_data;

        void m_parseExtData();
    };

    inline bool operator==(const m3u::Entry::ExtParamValue& a, const char* b) { return (a.data() == b); }
    inline bool operator==(const m3u::Entry::ExtParamValue& a, const std::string& b) { return (a.data() == b); }
    inline bool operator==(const char* a, const m3u::Entry::ExtParamValue& b) { return (a == b.data()); }
    inline bool operator==(const std::string& a, const m3u::Entry::ExtParamValue& b) { return (a == b.data()); }

    inline bool operator!=(const m3u::Entry::ExtParamValue& a, const char* b) { return (a.data() != b); }
    inline bool operator!=(const m3u::Entry::ExtParamValue& a, const std::string& b) { return (a.data() != b); }
    inline bool operator!=(const char* a, const m3u::Entry::ExtParamValue& b) { return (a != b.data()); }
    inline bool operator!=(const std::string& a, const m3u::Entry::ExtParamValue& b) { return (a != b.data()); }

    class M3U
    {
    public:
        M3U() noexcept : m_entries() {}
        M3U(const std::string& txt) : m_entries() { m_parse(txt.data(), txt.data() + txt.size()); }
        M3U(const char* p, const char* pEnd) : m_entries() { m_parse(p, pEnd); }
        virtual ~M3U() {}

        std::vector<m3u::Entry>& entries() { return m_entries; }
        const std::vector<m3u::Entry>& entries() const { return m_entries; }

        bool isEmpty() const { return m_entries.empty(); }
        bool isExt() const {}

        std::string serialize(const char* endOfLine = serializeEndOfLine) const;

    protected:
        std::vector<m3u::Entry> m_entries;

        void m_parse(const char* p, const char* pEnd);
    };
    
    class HLS : protected M3U
    {
    public:
        class AudioStream : protected m3u::Entry
        {
        public:
            AudioStream() = delete;
            AudioStream(const m3u::Entry& entry) : m3u::Entry(entry) {}
            virtual ~AudioStream() {}

            std::string serialize(const char* endOfLine = serializeEndOfLine) const { return m3u::Entry::serialize(endOfLine); }
        };

        class Subtitles : protected m3u::Entry
        {
        public:
            Subtitles() = delete;
            Subtitles(const m3u::Entry& entry) : m3u::Entry(entry), m_language(), m_forced(false), m_uri() { m_parse(); }
            virtual ~Subtitles() {}

            const std::string& language() const { return m_language; }
            bool forced() const { return m_forced; }
            const std::string& uri() const { return m_uri; }

            std::string serialize(const char* endOfLine = serializeEndOfLine) const { return m3u::Entry::serialize(endOfLine); }

        private:
            std::string m_language;
            bool m_forced;
            std::string m_uri;

            void m_parse();
        };

        class Stream : protected m3u::Entry
        {
        public:
            Stream() = delete;
            Stream(const m3u::Entry& entry) : m3u::Entry(entry) {}
            virtual ~Stream() {}

            std::string serialize(const char* endOfLine = serializeEndOfLine) const { return m3u::Entry::serialize(endOfLine); }
        };

    public:
        HLS() noexcept : M3U(), m_audioStreams(), m_subtitles(), m_streams(), m_otherEntries() {}
        HLS(const std::string& txt) : M3U(txt), m_audioStreams(), m_subtitles(), m_streams(), m_otherEntries() { m_parse(); }
        HLS(const char* p, const char* pEnd) : M3U(p, pEnd), m_audioStreams(), m_subtitles(), m_streams(), m_otherEntries() { m_parse(); }
        HLS(const M3U& m3u) : M3U(m3u), m_audioStreams(), m_subtitles(), m_streams(), m_otherEntries() { m_parse(); }
        virtual ~HLS() {}

        const std::vector<m3u::HLS::AudioStream>& audioStreams() const { return  m_audioStreams; }
        const std::vector<m3u::HLS::Subtitles>& subtitles() const { return  m_subtitles; }
        const std::vector<m3u::HLS::Stream>& streams() const { return  m_streams; }
        const std::vector<m3u::Entry>& otherEntries() const { return  m_otherEntries; }

        std::string serialize(const char* endOfLine = serializeEndOfLine) const { return m3u::M3U::serialize(endOfLine); }

    private:
        std::vector<m3u::HLS::AudioStream> m_audioStreams;
        std::vector<m3u::HLS::Subtitles> m_subtitles;
        std::vector<m3u::HLS::Stream> m_streams;

        std::vector<m3u::Entry> m_otherEntries;

        void m_parse();
    };
}


#endif // IG_MIDDLEWARE_M3U_H
