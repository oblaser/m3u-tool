/*
author          Oliver Blaser
date            17.12.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <string>
#include <vector>

#include "encoding.h"
#include "project.h"

#include <omw/windows/windows.h>


namespace
{
}



#ifdef OMW_PLAT_WIN

std::string enc::stou8(const wchar_t* str)
{
    return omw::windows::wstou8(str);
}

std::wstring enc::u8tos(const std::string& str)
{
    return omw::windows::u8tows(str);
}

#else // OMW_PLAT_WIN
#endif // OMW_PLAT_WIN
