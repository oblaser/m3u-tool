/*
author          Oliver Blaser
date            27.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <string_view>
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

namespace {

constexpr int ewiWidth = 10;

/*
bool isUrl(const std::string& uri)
{
    to be tested
    constexpr std::string_view http = "http://";
    constexpr std::string_view https = "https://";

    if (uri.substr(0, https.length()) == https) return true;
    if (uri.substr(0, http.length()) == http) return true;

    return false;
}
*/

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
        if ((text[0] == '#') && (text[1] == '#') && (text[2] == '#')) { format = true; }
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

void app::printInfo() { cout << omw::fgBrightCyan << std::left << std::setw(ewiWidth) << "info:" << omw::defaultForeColor; }

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
    // cout << omw::fgBrightWhite << title << omw::normal << endl;
    cout << title << endl;
}

#ifdef PRJ_DEBUG
void app::printDebug(const std::string& text)
{
    cout << omw::fgBrightMagenta << std::left << std::setw(ewiWidth) << "debug:" << omw::defaultForeColor;
    printFormattedText(text);
    cout << endl;
}
#endif // PRJ_DEBUG

void app::checkCreateOutDir(app::MessageCounter& msgCnt, const app::Flags& flags, const std::filesystem::path& outDirPath, const std::string& outDirArg)
{
    IMPLEMENT_FLAGS();

    if (fs::exists(outDirPath))
    {
        if (!fs::is_empty(outDirPath))
        {
            if (flags.force) { PRINT_WARNING_V("using non empty OUTDIR"); }
            else
            {
                const std::string msg = "###OUTDIR \"" + outDirArg + "\" is not empty";

                if (verbose)
                {
                    app::printInfo(msg);

                    if (2 == omw_::cli::choice("use non empty OUTDIR?")) { throw app::processor_exit(EC_USER_ABORT); }
                }
                else PRINT_ERROR_EXIT(msg, EC_OUTDIR_NOTEMPTY);
            }
        }
    }
    else
    {
        std::error_code ec;
        fs::create_directory(outDirPath, outDirPath.parent_path(), ec);

        if (!fs::exists(outDirPath)) PRINT_ERROR_EXIT("failed to create OUTDIR", EC_OUTDIR_NOTCREATED);
    }
}

void app::checkOutFile(app::MessageCounter& msgCnt, const app::Flags& flags, const std::filesystem::path& outFilePath, const std::string& fileDisplayPath,
                       const std::string& fileDisplayTitle)
{
    IMPLEMENT_FLAGS();

    if (fs::exists(outFilePath))
    {
        if (flags.force) { PRINT_WARNING_V("overwriting " + fileDisplayTitle); }
        else
        {
            const std::string msg = "###" + fileDisplayTitle + " \"" + fileDisplayPath + "\" already exists";

            if (verbose)
            {
                PRINT_INFO(msg);

                if (2 == omw_::cli::choice("overwrite " + fileDisplayTitle + "?")) { throw app::processor_exit(EC_USER_ABORT); }
            }
            else PRINT_ERROR_EXIT(msg, EC_OUTFILE_EXISTS);
        }
    }
}

m3u::M3U app::getFromUri(app::MessageCounter& msgCnt, const app::Flags& flags, const util::Uri& uri)
{
    IMPLEMENT_FLAGS();

    if (uri.isUrl())
    {
        util::Curl curl;

        const auto res = curl.httpGET(uri.string(), 60, 60);

        if (!res.good())
        {
            if (verbose) app::printInfo(res.toString());

            PRINT_ERROR("HTTP GET failed");
            PRINT_INFO_V("###M3U file: \"" + uri.string() + "\"");
            PROCESS_EXIT(EC_M3UFILE_NOTFOUND);
        }

        return res.data();
    }
    else
    {
        const auto file = enc::path(uri.path());

        if (!fs::exists(file))
        {
            PRINT_ERROR("M3U file not found");
            PRINT_INFO_V("###M3U file: \"" + uri.string() + "\"");
            PROCESS_EXIT(EC_M3UFILE_NOTFOUND);
        }

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
