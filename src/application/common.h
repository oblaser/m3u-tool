/*
author          Oliver Blaser
date            12.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#ifndef IG_APP_COMMON_H
#define IG_APP_COMMON_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>

#include "middleware/m3u.h"
#include "middleware/util.h"
#include "project.h"


#define IMPLEMENT_FLAGS()           \
const bool& quiet = flags.quiet;    \
bool ___verbose = flags.verbose;    \
const bool& verbose = ___verbose;   \
if (quiet) ___verbose = false;

#pragma region deprecated // TODO remove
#define ERROR_PRINT(msg)            \
{                                   \
    rcnt.incErrors();               \
    if (!quiet) app::printError(msg); \
}

#define INFO_PRINT(msg)         \
{                               \
    if (!quiet) app::printInfo(msg); \
}

#define WARNING_PRINT(msg)          \
{                                   \
    rcnt.incWarnings();             \
    if (!quiet) app::printWarning(msg); \
}

#define ERROR_PRINT_EC_THROWLINE(msg, EC_x) \
{                                   \
    if (!quiet) app::printError(msg); \
    r = EC_x;                       \
    throw (int)(__LINE__);          \
}
#pragma endregion

#define PRINT_ERROR(msg)        { if (!quiet) app::printError((msg)); }
#define PRINT_INFO(msg)         { if (!quiet) app::printInfo((msg)); }
#define PRINT_WARNING(msg)      { if (!quiet) app::printWarning((msg)); }

#define PRINT_INFO_V(msg)       { if (verbose) app::printInfo((msg)); }
#define PRINT_WARNING_V(msg)    { if (verbose) app::printWarning((msg)); }

// print error and abort process
#define PRINT_ERROR_EXIT(msg, ec)   \
{                                   \
    if (!quiet) app::printError((msg)); \
    throw app::processor_exit((ec)); \
}



// TODO move to ns app
enum ERRORCODE // https://tldp.org/LDP/abs/html/exitcodes.html / on MSW are no preserved codes
{
    EC_OK = 0,
    EC_ERROR = 1,

    EC__begin_ = 79,

    EC_MODULE_UNKNOWN = EC__begin_,
    EC_M3UFILE_NOTFOUND,
    EC_OUTDIR_NOTFOUND,
    EC_OUTDIR_NOTEMPTY,
    EC_OUTDIR_NOTCREATED,
    EC_OUTFILE_EXISTS,
    EC_STREAM_EMPTY,

    EC_USER_ABORT,

    //EC_ENCCONV,

    EC__end_,

    EC__max_ = 113
};
static_assert(EC__end_ <= EC__max_, "too many error codes defined");


namespace app
{
    struct Flags
    {
        Flags() = delete;

        Flags(bool force_, bool quiet_, bool verbose_)
            : force(force_), quiet(quiet_), verbose(verbose_)
        {}

        bool force;
        bool quiet;
        bool verbose;
    };

    // throwable exit code, catched by the processor
    class processor_exit : public std::runtime_error
    {
    public:
        processor_exit() : std::runtime_error("processor_exit " + std::to_string(EC_ERROR)), m_ec(EC_ERROR) {}
        explicit processor_exit(int ec) : std::runtime_error("processor_exit " + std::to_string(ec)), m_ec(ec) {}
        virtual ~processor_exit() {}

        virtual int ec() const { return m_ec; }

    private:
        int m_ec;
    };

    void printFormattedText(const std::string& text);
    void printFormattedLine(const std::string& text);
    void printError(const std::string& text);
    void printInfo();
    void printInfo(const std::string& text);
    void printWarning(const std::string& text);
    void printTitle(const std::string& title);
#ifdef PRJ_DEBUG
    void printDebug(const std::string& text);
#endif // PRJ_DEBUG

    void checkCreateOutDir(const app::Flags& flags, const std::filesystem::path& outDirPath, const std::string& outDirArg);
    static inline void checkCreateOutDir(const app::Flags& flags, const std::filesystem::path& outDirPath) { app::checkCreateOutDir(flags, outDirPath, outDirPath.u8string()); }

    m3u::M3U getFromUri(const app::Flags& flags, const std::string& uri);

#ifdef PRJ_DEBUG
    void dbg_rm_outDir(const std::filesystem::path& outDir);
#endif // PRJ_DEBUG
}


#endif // IG_APP_COMMON_H
