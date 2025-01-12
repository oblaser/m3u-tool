/*
author          Oliver Blaser
date            07.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#ifndef IG_APP_PATH_H
#define IG_APP_PATH_H

#include <string>
#include <vector>

#include "application/cliarg.h"
#include "application/common.h"


namespace app {

int path(const app::Args& args, const app::Flags& flags);

}


#endif // IG_APP_PATH_H
