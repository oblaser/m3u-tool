/*
author          Oliver Blaser
date            12.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
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
    extern const char* const ext_str;
    extern const char* const extm3u_str; // header of extended M3U
    extern const char* const extenc_str;
    extern const char* const extinf_str;
    extern const char* const ext_x_media_str;
    extern const char* const ext_x_stream_inf_str;

    extern const char* const serialiseEndOfLine;

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

            void setData(const std::string& data) { m_data = data; }

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
            ExtParameter(const std::string& key, const std::string& value) : std::pair<std::string, m3u::Entry::ExtParamValue>(key, value), m_validity(true) {}
            ExtParameter(const std::string& key, const ExtParamValue& value) : std::pair<std::string, m3u::Entry::ExtParamValue>(key, value), m_validity(true) {}
            virtual ~ExtParameter() {}

            const std::string& key() const { return first; }
            m3u::Entry::ExtParamValue& value() { return second; }
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
        explicit Entry(const std::string& data, const std::string& ext = std::string()) : m_ext(ext), m_extParam(), m_data(data) { m_parseExtData(); }
        virtual ~Entry() {}

        const std::string& ext() const { return m_ext; }
        const std::string& data() const { return m_data; }

        bool extIs(const std::string& extBaseStr) const;

        const ExtParamContainer& extParam() const { return m_extParam; }

        bool isEmpty() const { return (m_ext.empty() && m_data.empty()); }
        bool isComment() const { return (!m_data.empty() && (m_data[0] == '#')); }
        bool isExtension() const { return (!m_ext.empty() && m_data.empty()); }
        bool isResource() const { return (!m_data.empty() && (m_data[0] != '#')); } // is resource (with or without extension)
        bool hasExtension() const { return !m_ext.empty(); }
        bool isRegularRes() const { return (isResource() && !hasExtension()); }

        virtual void setExt(const std::string& ext) { m_ext = ext; m_parseExtData(); }
        virtual void setData(const std::string& data) { m_data = data; }

        std::string serialise(const char* endOfLine = serialiseEndOfLine) const;

    protected:
        m3u::Entry::ExtParamContainer m_extParam;

    private:
        std::string m_ext;
        std::string m_data;

        void m_parseExtData();
    };

    inline bool operator==(const m3u::Entry::ExtParamValue& a, const char* b) { return (a.data() == b); }
    inline bool operator==(const m3u::Entry::ExtParamValue& a, const std::string& b) { return (a.data() == b); }
    inline bool operator==(const char* a, const m3u::Entry::ExtParamValue& b) { return (a == b.data()); }
    inline bool operator==(const std::string& a, const m3u::Entry::ExtParamValue& b) { return (a == b.data()); }
    inline bool operator!=(const m3u::Entry::ExtParamValue& a, const char* b) { return !(a == b); }
    inline bool operator!=(const m3u::Entry::ExtParamValue& a, const std::string& b) { return !(a == b); }
    inline bool operator!=(const char* a, const m3u::Entry::ExtParamValue& b) { return !(a == b); }
    inline bool operator!=(const std::string& a, const m3u::Entry::ExtParamValue& b) { return !(a == b); }

    inline bool operator==(const m3u::Entry& a, const m3u::Entry& b) { return ((a.data() == b.data()) && (a.ext() == b.ext())); }
    inline bool operator!=(const m3u::Entry& a, const m3u::Entry& b) { return !(a == b); }

    class M3U
    {
    public:
        M3U() noexcept : m_entries() {}
        M3U(const std::string& txt) : m_entries() { m_parse(txt.data(), txt.data() + txt.size()); }
        M3U(const char* p, const char* pEnd) : m_entries() { m_parse(p, pEnd); }
        virtual ~M3U() {}

        void add(const m3u::Entry& entry) { m_entries.push_back(entry); }

        std::vector<m3u::Entry>& entries() { return m_entries; }
        const std::vector<m3u::Entry>& entries() const { return m_entries; }

        bool isEmpty() const { return m_entries.empty(); }
        bool isExtended() const { return (isEmpty() ? false : (m_entries[0] == m3u::Entry("", m3u::extm3u_str))); }

        std::string serialise(const char* endOfLine = serialiseEndOfLine) const;

    protected:
        std::vector<m3u::Entry> m_entries;

        void m_parse(const char* p, const char* pEnd);
    };
    
    class HLS : protected M3U
    {
    public:
        class AudioStream : public m3u::Entry
        {
        public:
            AudioStream() = delete;
            AudioStream(const m3u::Entry& entry) : m3u::Entry(entry) {}
            virtual ~AudioStream() {}

            const std::string uri() const;

            void setUri(const std::string& uri);
        };

        class Subtitles : public m3u::Entry
        {
        public:
            Subtitles() = delete;
            Subtitles(const m3u::Entry& entry) : m3u::Entry(entry), m_language(), m_forced(false), m_uri() { m_parse(); }
            virtual ~Subtitles() {}

            const std::string& language() const { return m_language; }
            bool forced() const { return m_forced; }
            const std::string& uri() const { return m_uri; }

            virtual void setExt(const std::string& ext) { m3u::Entry::setExt(ext); m_parse(); }

        private:
            std::string m_language;
            bool m_forced;
            std::string m_uri;

            void m_parse();
        };

        class Stream : public m3u::Entry
        {
        public:
            Stream() = delete;
            Stream(const m3u::Entry& entry) : m3u::Entry(entry), m_resolutionHeight(-1) { m_parse(); }
            virtual ~Stream() {}

            m3u::Entry::ExtParameter resolutionExtParam() const;
            int resolutionHeight() const { return m_resolutionHeight; }

            virtual void setExt(const std::string& ext) { m3u::Entry::setExt(ext); m_parse(); }

        private:
            int m_resolutionHeight;

            void m_parse();
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

        std::string serialise(const char* endOfLine = serialiseEndOfLine) const { return m3u::M3U::serialise(endOfLine); }

    private:
        std::vector<m3u::HLS::AudioStream> m_audioStreams;
        std::vector<m3u::HLS::Subtitles> m_subtitles;
        std::vector<m3u::HLS::Stream> m_streams;

        std::vector<m3u::Entry> m_otherEntries;

        void m_parse();
    };
}


#endif // IG_MIDDLEWARE_M3U_H
