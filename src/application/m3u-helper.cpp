/*
author          Oliver Blaser
date            04.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include "defines.h"
#include "m3u-helper.h"
#include "middleware/curl-helper.h"
#include "middleware/encoding-helper.h"
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

    std::string readFile(const fs::path& file)
    {
        std::stringstream txt;

        std::ifstream ifs;
        ifs.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
        ifs.open(file, std::ios::in | std::ios::binary); // binary so that no conversations happen
        txt << ifs.rdbuf();
        ifs.close();

        return txt.str();
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
        const auto file = enc::path(uri);

        if (!fs::exists(file)) ERROR_PRINT_EC_THROWLINE("M3U file not found", EC_M3UFILE_NOT_FOUND);

        return readFile(file);
    }
}
