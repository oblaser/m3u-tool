/*
author          Oliver Blaser
date            07.05.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_APP_EXPORT_H
#define IG_APP_EXPORT_H

#include <string>
#include <vector>


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
}


namespace exprt // no, thats not a typo, "export" is a keyword
{
    int process(const std::string& m3uFile, const std::string& outDir, const app::Flags& flags);
}


#endif // IG_APP_EXPORT_H
