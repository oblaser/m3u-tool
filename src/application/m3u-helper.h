/*
author          Oliver Blaser
date            26.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_APP_M3UHELPER_H
#define IG_APP_M3UHELPER_H

#include <cstddef>
#include <cstdint>
#include <string>

#include "cliarg.h"
#include "middleware/m3u.h"


namespace app
{
    m3u::M3U getFromUri(int& r, const app::Flags& flags, const std::string& uri);
}


#endif // IG_APP_M3UHELPER_H
