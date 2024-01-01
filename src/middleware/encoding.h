/*
author          Oliver Blaser
date            17.12.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_MIDDLEWARE_ENCODING_H
#define IG_MIDDLEWARE_ENCODING_H

#include <cstddef>
#include <cstdint>
#include <string>

#include <omw/defs.h>


namespace enc
{
#ifdef OMW_PLAT_WIN

    // may throw, see https://static.oblaser.ch/omw/doc/0.2.1-beta/namespaceomw_1_1windows.html#omw_windows_strConv_infoText (TODO update link)
    std::string stou8(const wchar_t* str);
    static inline std::string stou8(const std::wstring& str) { return stou8(str.c_str()); }
    std::wstring u8tos(const std::string& str);

#else // OMW_PLAT_WIN

    // no conversation needed, OS is using UTF-8
    static inline const std::string& stou8(const std::string& str) { return str; }
    static inline const std::string& u8tos(const std::string& str) { return str; }

#endif // OMW_PLAT_WIN
}


#endif // IG_MIDDLEWARE_ENCODING_H
