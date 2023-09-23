/*
author          Oliver Blaser
date            23.09.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_APP_EXPORT_H
#define IG_APP_EXPORT_H

#include <string>
#include <vector>

#include "application/cliarg.h"


namespace app
{
    // no, that's not a typo, "export" is a keyword
    int exprt(const app::Args& args, const app::Flags& flags, const std::string& m3uFile, const std::string& outDir);
}


#endif // IG_APP_EXPORT_H
