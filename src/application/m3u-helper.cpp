/*
author          Oliver Blaser
date            27.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "defines.h"
#include "m3u-helper.h"
#include "middleware/curl-helper.h"
#include "middleware/util.h"
#include "project.h"

#include <omw/io/file.h>
#include <omw/string.h>


namespace fs = std::filesystem;

namespace
{
    bool isUrl(const std::string& uri)
    {
        const auto tmp = uri.substr(0, 10);
        return (omw::contains(tmp, "https://") || omw::contains(tmp, "http://"));
    }

    std::string readFile(const std::string& file)
    {
        const auto fileIf = omw::io::TxtFileInterface(file);
        fileIf.openRead();
        const size_t fileSize = fileIf.size();

        std::string ___txt(fileSize, '#');
        const std::string& txt = ___txt;

        fileIf.read(___txt.data(), ___txt.size());

        return txt;
    }
}



m3u::M3U app::getFromUri(int& r, const app::Flags& flags, const std::string& uri)
{
    IMPLEMENT_FLAGS();

    if (isUrl(uri))
    {
        util::Curl curl;

        const auto res = curl.httpGET(uri, 60, 60);

        if (!res.good())
        {
            if (verbose) util::printInfo(res.toString());
            ERROR_PRINT_EC_THROWLINE("HTTP GET failed", EC_M3UFILE_NOT_FOUND);
        }

        return res.data();
    }
    else
    {
        if (!fs::exists(uri)) ERROR_PRINT_EC_THROWLINE("M3U file not found", EC_M3UFILE_NOT_FOUND);

        return readFile(uri);
    }
}
