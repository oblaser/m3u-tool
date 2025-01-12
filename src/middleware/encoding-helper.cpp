/*
author          Oliver Blaser
date            04.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <filesystem>
#include <string>
#include <vector>

#include "encoding-helper.h"
#include "project.h"

#include <omw/windows/windows.h>


namespace fs = std::filesystem;

namespace {}

#ifdef OMW_PLAT_WIN
#include <Windows.h>
#define INITIAL_BUFFER_SIZE  (300)
#define INITIAL_INC_SIZE     (50)
#define acptows_fnNamePrefix "omw_::acptows: "
namespace omw_ {

std::wstring acptows(const char* str)
{
    using size_type = std::vector<wchar_t>::size_type;
    static_assert(sizeof(size_type) >= sizeof(int), "invalid integer sizes"); // should always be true on Windows

    std::vector<wchar_t> buffer;

    if (str)
    {
        constexpr size_type initialSize = INITIAL_BUFFER_SIZE;
        size_type incSize = INITIAL_INC_SIZE;
        int res;
        constexpr int maxDestSize = INT_MAX;
        int destSize;

        buffer = std::vector<wchar_t>(initialSize, L'\0');

        do {
            if (buffer.size() > static_cast<size_type>(maxDestSize)) destSize = maxDestSize;
            else destSize = static_cast<int>(buffer.size());

            res = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str, -1, buffer.data(), destSize);

            if (res <= 0)
            {
                const DWORD err = GetLastError();

                if (err == ERROR_INSUFFICIENT_BUFFER)
                {
                    if (destSize == maxDestSize) throw std::length_error(acptows_fnNamePrefix OMWi_DISPSTR("src is too long"));

                    buffer = std::vector<wchar_t>(initialSize + incSize, L'\0');
                    incSize *= 2;
                }
                else if (err == ERROR_INVALID_PARAMETER) { throw std::invalid_argument(acptows_fnNamePrefix OMWi_DISPSTR("invalid arguments")); }
                else if (err == ERROR_NO_UNICODE_TRANSLATION)
                {
                    throw omw::windows::invalid_unicode(acptows_fnNamePrefix OMWi_DISPSTR("invalid unicode in src"));
                }
                else if (err == ERROR_INVALID_FLAGS) { throw std::runtime_error(acptows_fnNamePrefix OMWi_DISPSTR("internal error (invalid flags)")); }
                else if (res < 0)
                {
                    throw std::runtime_error(acptows_fnNamePrefix OMWi_DISPSTR("Windows API error, MultiByteToWideChar() ") + std::to_string(res) +
                                             OMWi_DISPSTR(", GetLastError() ") + std::to_string(err));
                }
                else { throw std::runtime_error(acptows_fnNamePrefix OMWi_DISPSTR("internal error")); }
            }
        }
        while (res <= 0);
    }
    else throw std::invalid_argument(acptows_fnNamePrefix OMWi_DISPSTR("src is NULL"));

    return buffer.data();
}

static inline std::wstring acptows(const std::string& str) { return omw_::acptows(str.c_str()); }

} // namespace omw_

#endif // OMW_PLAT_WIN



#ifdef OMW_PLAT_WIN

std::string enc::acptou8(const std::string& str) { return omw::windows::wstou8(omw_::acptows(str)); }

#else  // OMW_PLAT_WIN
#endif // OMW_PLAT_WIN
