/*
author          Oliver Blaser
date            18.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_APP_DEFINES_H
#define IG_APP_DEFINES_H

#include <cstddef>
#include <cstdint>


#define IMPLEMENT_FLAGS()           \
const bool quiet = flags.quiet;     \
bool ___verbose = flags.verbose;    \
const bool& verbose = ___verbose;   \
if (quiet) ___verbose = false;

#define ERROR_PRINT(msg)            \
{                                   \
    rcnt.incErrors();               \
    if (!quiet) util::printError(msg); \
}

#define INFO_PRINT(msg)         \
{                               \
    if (!quiet) util::printInfo(msg); \
}

#define WARNING_PRINT(msg)          \
{                                   \
    rcnt.incWarnings();             \
    if (!quiet) util::printWarning(msg); \
}

#define ERROR_PRINT_EC_THROWLINE(msg, EC_x) \
{                                   \
    if (!quiet) util::printError(msg); \
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

    EC_USER_ABORT,

    EC__end_,

    EC__max_ = 113
};
static_assert(EC__end_ <= EC__max_, "too many error codes defined");


namespace app
{
}


#endif // IG_APP_DEFINES_H
