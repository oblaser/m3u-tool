/*
author          Oliver Blaser
date            07.05.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_PROJECT_H
#define IG_PROJECT_H

#include <omw/defs.h>
#include <omw/version.h>


namespace prj
{
    const char* const appDirName_windows = "m3u-tool";
    const char* const appDirName_unix = "m3u-tool";

#ifdef OMW_PLAT_WIN
    const char* const appDirName = appDirName_windows;
#else
    const char* const appDirName = appDirName_unix;
#endif

    const char* const appName = "M3U Tool";
    const char* const exeName = "m3u-tool"; // eq to the linker setting

    const char* const website = "https://github.com/oblaser/m3u-tool";

    const omw::Version version(0, 1, 2, "");
}


#ifdef OMW_DEBUG
#define PRJ_DEBUG (1)
#else
#undef PRJ_DEBUG
#endif


#endif // IG_PROJECT_H
