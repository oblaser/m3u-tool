/*
author          Oliver Blaser
date            05.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#ifndef IG_APP_COMMON_H
#define IG_APP_COMMON_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

#include "middleware/m3u.h"
#include "middleware/util.h"
#include "project.h"


#define IMPLEMENT_FLAGS()           \
const bool quiet = flags.quiet;     \
bool ___verbose = flags.verbose;    \
const bool& verbose = ___verbose;   \
if (quiet) ___verbose = false;

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


// TODO move to ns app
enum ERRORCODE // https://tldp.org/LDP/abs/html/exitcodes.html / on MSW are no preserved codes
{
    EC_OK = 0,
    EC_ERROR = 1,

    EC__begin_ = 79,

    EC_MODULE_UNKNOWN = EC__begin_,
    EC_OUTDIR_NOTEMPTY,
    EC_OUTDIR_NOTCREATED,
    EC_M3UFILE_NOT_FOUND,

    EC_STREAM_EMPTY,

    EC_USER_ABORT,

    EC_ENCCONV,

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

    void printFormattedText(const std::string& text);
    void printFormattedLine(const std::string& text);
    void printError(const std::string& text);
    void printInfo();
    void printInfo(const std::string& text);
    void printWarning(const std::string& text);
    void printTitle(const std::string& title);

    void checkCreateOutDir(util::ResultCounter& rcnt, const app::Flags& flags, const std::filesystem::path& outDirPath, const std::string& outDirArg);

    m3u::M3U getFromUri(int& r, const app::Flags& flags, const std::string& uri);

#if defined(PRJ_DEBUG)
    void dbg_rm_outDir(const std::filesystem::path& outDir);
#endif
}


#endif // IG_APP_COMMON_H
