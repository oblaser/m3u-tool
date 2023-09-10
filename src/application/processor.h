/*
author          Oliver Blaser
date            19.08.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_APP_PROCESSOR_H
#define IG_APP_PROCESSOR_H

#include <string>
#include <vector>

#include "application/cliarg.h"


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

    int process(const app::Args& args);
}


#endif // IG_APP_PROCESSOR_H
