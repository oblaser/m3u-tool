/*
author          Oliver Blaser
date            05.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "common.h"
#include "middleware/curl-helper.h"
#include "middleware/encoding-helper.h"
#include "middleware/util.h"
#include "project.h"

#include <omw/cli.h>
#include <omw/string.h>


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace
{
    constexpr int ewiWidth = 10;

    bool isUrl(const std::string& uri)
    {
        const auto tmp = uri.substr(0, 10);
        return (omw::contains(tmp, "https://") || omw::contains(tmp, "http://"));
    }
}



// 
// "### normal "quoted bright" white"
// "### normal @just bright@ white"
// 
void app::printFormattedText(const std::string& text)
{
    bool format = false;

    if (text.length() > 5)
    {
        if ((text[0] == '#') &&
            (text[1] == '#') &&
            (text[2] == '#')
            )
        {
            format = true;
        }
    }

    if (format)
    {
        bool on = false;

        size_t i = 3;

        while (i < text.length())
        {
            if (text[i] == '\"')
            {
                if (on)
                {
                    cout << omw::defaultForeColor;
                    cout << text[i];
                    on = false;
                }
                else
                {
                    cout << text[i];
                    cout << omw::fgBrightWhite;
                    on = true;
                }
            }
            else if (text[i] == '@')
            {
                if (on)
                {
                    cout << omw::defaultForeColor;
                    on = false;
                }
                else
                {
                    cout << omw::fgBrightWhite;
                    on = true;
                }
            }
            else cout << text[i];

            ++i;
        }

        cout << omw::defaultForeColor;
    }
    else cout << text;
}

void app::printFormattedLine(const std::string& text)
{
    printFormattedText(text);
    cout << endl;
}

void app::printError(const std::string& text)
{
    cout << omw::fgBrightRed << std::left << std::setw(ewiWidth) << "error:" << omw::defaultForeColor;
    printFormattedText(text);
    cout << endl;
}

void app::printInfo()
{
    cout << omw::fgBrightCyan << std::left << std::setw(ewiWidth) << "info:" << omw::defaultForeColor;
}

void app::printInfo(const std::string& text)
{
    printInfo();
    printFormattedText(text);
    cout << endl;
}

void app::printWarning(const std::string& text)
{
    cout << omw::fgBrightYellow << std::left << std::setw(ewiWidth) << "warning:" << omw::defaultForeColor;
    printFormattedText(text);
    cout << endl;
}

void app::printTitle(const std::string& title)
{
    //cout << omw::fgBrightWhite << title << omw::normal << endl;
    cout << title << endl;
}

void app::checkCreateOutDir(util::ResultCounter& rcnt, const app::Flags& flags, const std::filesystem::path& outDirPath, const std::string& outDirArg)
{
    IMPLEMENT_FLAGS();

    int r; // dummy, can be removed if proper error handling (process_error exception instead of throwing line number) is implemented

    if (fs::exists(outDirPath))
    {
        if (!fs::is_empty(outDirPath))
        {
            if (flags.force)
            {
                if (verbose) WARNING_PRINT("using non empty OUTDIR");
            }
            else
            {
                const std::string msg = "###OUTDIR \"" + outDirArg + "\" is not empty";

                if (verbose)
                {
                    app::printInfo(msg);

                    if (2 == omw_::cli::choice("use non empty OUTDIR?"))
                    {
                        r = EC_USER_ABORT;
                        throw (int)(__LINE__);
                    }
                }
                else ERROR_PRINT_EC_THROWLINE(msg, EC_OUTDIR_NOTEMPTY);
            }
        }
    }
    else
    {
        fs::create_directories(outDirPath);

        if (!fs::exists(outDirPath)) ERROR_PRINT_EC_THROWLINE("failed to create OUTDIR", EC_OUTDIR_NOTCREATED);
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
            if (verbose) app::printInfo(res.toString());
            ERROR_PRINT_EC_THROWLINE("HTTP GET failed", EC_M3UFILE_NOT_FOUND);
        }

        return res.data();
    }
    else
    {
        const auto file = enc::path(uri);

        if (!fs::exists(file)) ERROR_PRINT_EC_THROWLINE("M3U file not found", EC_M3UFILE_NOT_FOUND);

        return util::readFile(file);
    }
}

#if defined(PRJ_DEBUG)
void app::dbg_rm_outDir(const fs::path& outDir)
{
    try
    {
        const auto n = fs::remove_all(outDir);
        cout << omw::fgBrightBlack << "rm OUTDIR: " << n << " items deleted" << omw::fgDefault << endl;
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        cout << omw::fgBrightMagenta << __FUNCTION__ << omw::fgDefault << endl;
        throw ex;
    }
    catch (const std::system_error& ex)
    {
        cout << omw::fgBrightMagenta << __FUNCTION__ << omw::fgDefault << endl;
        throw ex;
    }
    catch (const std::exception& ex)
    {
        cout << omw::fgBrightMagenta << __FUNCTION__ << omw::fgDefault << endl;
        throw ex;
    }
}
#endif
