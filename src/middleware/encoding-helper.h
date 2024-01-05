/*
author          Oliver Blaser
date            04.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#ifndef IG_MIDDLEWARE_ENCODINGHELPER_H
#define IG_MIDDLEWARE_ENCODINGHELPER_H

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

#include <omw/defs.h>
#include <omw/windows/windows.h>


namespace enc
{
#ifdef OMW_PLAT_WIN

    // may throw, see https://static.oblaser.ch/omw/doc/0.2.1-beta/namespaceomw_1_1windows.html#omw_windows_strConv_infoText (TODO update link)
    static inline std::string stou8(const std::wstring& str) { return omw::windows::wstou8(str); }
    static inline std::wstring u8tos(const std::string& str) { return omw::windows::u8tows(str); }
    std::string acptou8(const std::string& str);

#else // OMW_PLAT_WIN

    // no conversation needed, OS is using UTF-8
    static inline const std::string& stou8(const std::string& str) { return str; }
    static inline const std::string& u8tos(const std::string& str) { return str; }
    static inline const std::string& acptou8(const std::string& str) { return str; }

#endif // OMW_PLAT_WIN

    static inline std::filesystem::path path(const std::string& path_u8) { return enc::u8tos(path_u8); }
}


#endif // IG_MIDDLEWARE_ENCODINGHELPER_H
