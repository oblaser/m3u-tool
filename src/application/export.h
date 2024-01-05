/*
author          Oliver Blaser
date            05.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#ifndef IG_APP_EXPORT_H
#define IG_APP_EXPORT_H

#include <string>
#include <vector>

#include "application/cliarg.h"
#include "application/common.h"


namespace app
{
    // no, that's not a typo, "export" is a keyword
    int exprt(const app::Args& args, const app::Flags& flags);
}


#endif // IG_APP_EXPORT_H
