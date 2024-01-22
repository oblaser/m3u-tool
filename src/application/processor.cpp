/*
author          Oliver Blaser
date            12.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "application/common.h"
#include "application/export.h"
#include "application/path.h"
#include "application/vstreamdl.h"
#include "middleware/encoding-helper.h"
#include "middleware/util.h"
#include "processor.h"
#include "project.h"

#include <omw/cli.h>
#include <omw/string.h>
#include <omw/vector.h>


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace
{
#ifdef PRJ_DEBUG
    const std::string magentaDebugStr = "\033[95mDEBUG\033[39m";
#endif
}



int app::process(const app::Args& args)
{
    int r = EC_ERROR;
    
    const auto flags = app::Flags(args.containsForce(),
                args.containsQuiet(),
                args.containsVerbose());

    IMPLEMENT_FLAGS();

    try
    {
#if defined(OMW_PLAT_WIN) && defined(PRJ_DEBUG) && 0 // tests with UTF-8 path on Windows
        {
            // https://stackoverflow.com/questions/45401822/how-to-convert-filesystem-path-to-string

            //const std::string str_u8 = "\xc3\xae-\xc3\xab-\xC5\xA4";
            const std::string str_u8 = "\xc3\xae-\xc3\xab";
            const std::string str_mb = "\xEE-\xEB";
            const std::wstring str_w = enc::u8tos(str_u8);
            const std::vector<char> bin_u8(str_u8.begin(), str_u8.end());
            std::vector<wchar_t> ___bin_u8w;
            for (const auto& c : str_u8) { ___bin_u8w.push_back((uint8_t)c); }
            const std::vector<wchar_t>& bin_u8w = ___bin_u8w;
            const std::vector<wchar_t> bin_w(str_w.begin(), str_w.end());
            //const auto binstr_u8 = omw::toHexStr(bin_u8);

            // no encoding conversation done
            const auto p_u8 = fs::path(str_u8); // contains the UTF-8 encoded string as wchar_t (like uint8_t to uint16_t), failes find existing file
            const auto p_w = fs::path(str_w); // natively encoded, can find existing files

            const std::wstring pnative_u8 = p_u8.native(); // c_str()
            const std::wstring pnative_w = p_w.native(); // c_str()
            const bool pnative_ok = ((bin_u8w == std::vector<wchar_t>(pnative_u8.begin(), pnative_u8.end())) && (bin_w == std::vector<wchar_t>(pnative_w.begin(), pnative_w.end())));

            const std::string pstr_u8 = p_u8.string();
            const std::string pstr_w = p_w.string(); // converts to the current Windows code page
            const bool pstr_ok = ((pstr_u8 == str_u8) && (pstr_w == str_mb));

            const std::string pu8str_u8 = p_u8.u8string();
            const std::string pu8str_w = p_w.u8string();
            const bool pu8str_ok = ((pu8str_u8 == enc::stou8(std::wstring(bin_u8w.begin(), bin_u8w.end()))) && (bin_u8 == std::vector<char>(pu8str_w.begin(), pu8str_w.end())));

            // debugger display
            const std::wstring wpu8str_u8 = enc::u8tos(pu8str_u8);
            const std::wstring wpu8str_w = enc::u8tos(pu8str_w);
        }
#endif

        if (args.raw.at(0) == "export") r = app::exprt(args, flags);
        else if (args.raw.at(0) == "path") r = app::path(args, flags);
        else if (args.raw.at(0) == "vstreamdl") r = app::vstreamdl(args, flags);
        else if (args.raw.at(0) == "parse")
        {
            const auto uri = util::Uri(args.raw.at(1));
            const m3u::M3U m3u = app::getFromUri(flags, uri);

            for (const auto& e : m3u.entries())
            {
                if (e.isRegularRes()) cout << omw::fgBrightCyan << "R " << omw::fgBrightWhite << e.data() << omw::fgDefault << endl;
                else if (e.isComment()) cout << omw::fgBrightBlack << "C " << omw::fgDefault << e.data() << endl;
                else if (e.isExtension()) cout << omw::fgGreen << "X " << omw::fgDefault << e.ext() << endl;
                else if (e.hasExtension()) cout << omw::fgBrightYellow << "X " << omw::fgDefault << e.ext() << "\n  " << omw::fgBrightWhite << e.data() << omw::fgDefault << endl;
                else cout << omw::fgBrightRed << "E " << omw::fgDefault << e.data() << endl;
            }

#if defined(PRJ_DEBUG) && 1
            cout << "\n===================================================\n" << m3u.serialise() << "<EOF>==============================================" << endl;
#endif
            r = EC_OK;
        }
        else PRINT_ERROR_EXIT("unknown module", EC_MODULE_UNKNOWN);
    }
    catch (const app::processor_exit& ex)
    {
        r = ex.ec();

        if (verbose)
        {
            if (r == EC_USER_ABORT) cout << omw::fgRed << "aborted by user" << omw::defaultForeColor << endl;
            else cout << omw::fgBrightRed << "failed" << omw::defaultForeColor << endl;
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            app::printError("fatal error");
            cout << "    path1: " << ex.path1().u8string() << endl;
            cout << "    path2: " << ex.path2().u8string() << endl;
            cout << "    cat:   " << enc::acptou8(ex.code().category().name()) << endl;
            cout << "    code:  " << ex.code().value() << endl;
            cout << "    msg:   " << enc::acptou8(ex.code().message()) << endl;
            cout << "    what:  " << enc::acptou8(ex.what()) << endl;
        }
    }
    catch (const std::system_error& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            app::printError("fatal error");
            cout << "    cat:   " << enc::acptou8(ex.code().category().name()) << endl;
            cout << "    code:  " << ex.code().value() << endl;
            cout << "    msg:   " << enc::acptou8(ex.code().message()) << endl;
            cout << "    what:  " << enc::acptou8(ex.what()) << endl;
        }
    }
    catch (const std::exception& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            app::printError("fatal error");
            cout << "    what:  " << enc::acptou8(ex.what()) << endl;
        }
    }
    catch (const int& ex)
    {
        r = EC_ERROR;
        if (!quiet) app::printError("fatal error (" + std::to_string(ex) + ")");
    }
    catch (...)
    {
        r = EC_ERROR;
        if (!quiet) app::printError("unspecified fatal error");
    }

    //if (r == EC_USER_ABORT) r = EC_OK;

    return r;
}
