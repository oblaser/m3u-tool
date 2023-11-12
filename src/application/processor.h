/*
author          Oliver Blaser
date            12.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_APP_PROCESSOR_H
#define IG_APP_PROCESSOR_H

#include <string>
#include <vector>

#include "application/cliarg.h"


namespace app
{
    int process(const app::Args& args);
}


#endif // IG_APP_PROCESSOR_H
