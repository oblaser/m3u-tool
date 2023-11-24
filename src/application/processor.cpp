/*
author          Oliver Blaser
date            23.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
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

#include "application/export.h"
#include "defines.h"
#include "middleware/m3u.h"
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
    bool equivalent(const std::vector<fs::path>& inDirs, const fs::path& outDir)
    {
        bool r = false;

        for (size_t i = 0; (i < inDirs.size()) && !r; ++i)
        {
            if (fs::exists(inDirs[i])) r = fs::equivalent(inDirs[i], outDir);
        }

        return r;
    }

#ifdef PRJ_DEBUG
    const std::string magentaDebugStr = "\033[95mDEBUG\033[39m";
#endif
}



int app::process(const app::Args& args)
{
    int r = EC_OK; // set to OK because of catch(...)

    const auto flags = app::Flags(args.containsForce(),
                args.containsQuiet(),
                args.containsVerbose());

    IMPLEMENT_FLAGS();

    try
    {
        if (args.raw.at(0) == "export")
        {
            r = app::exprt(args, flags);
        }
        else if (args.raw.at(0) == "parse")
        {
            const std::string m3uFile = args.raw.at(1);

            if (!fs::exists(m3uFile)) ERROR_PRINT_EC_THROWLINE("M3U file not found", EC_M3UFILE_NOT_FOUND);

            const auto m3u = m3u::M3U(m3uFile);

            for (const auto& e : m3u.entries())
            {
                if (e.isRegularRes()) cout << omw::fgBrightCyan << "R " << omw::fgDefault << e.path() << endl;
                else if (e.isComment()) cout << omw::fgBrightBlack << "C " << omw::fgDefault << e.data() << endl;
                else if (e.isExtension()) cout << omw::fgGreen << "X " << omw::fgDefault << e.ext() << endl;
                else if (e.hasExtension()) cout << omw::fgBrightYellow << "X " << omw::fgDefault << e.ext() << " \"" << omw::fgBrightWhite << e.path() << omw::fgDefault << "\"" << endl;
                else cout << omw::fgBrightRed << "E " << omw::fgDefault << e.path() << endl;
            }

#if defined(PRJ_DEBUG) && 1
            cout << "\n===================================================\n" << m3u.serialize() << "<EOF>==============================================" << endl;
#endif
        }
        else if (args.raw.at(0) == "vstreamdl")
        {
            const std::string m3uFile = args.raw.at(1);

            if (!fs::exists(m3uFile)) ERROR_PRINT_EC_THROWLINE("M3U file not found", EC_M3UFILE_NOT_FOUND);

            const auto hls = m3u::HLS(m3uFile);

            const std::string stemFileName = fs::path(m3uFile).stem().string();

            std::string srtScript = "\nmkdir subs\n\n";

            for (const auto& st : hls.subtitles())
            {
                if (!st.uri().empty())
                {
                    const std::string filename = "./subs/" + stemFileName + "-" + st.language() + (st.forced() ? "-forced" : "") + ".srt";

                    srtScript += "ffmpeg -i \"" + st.uri() + "\" -scodec srt -loglevel warning \"" + filename + "\"\n";
                    srtScript += "echo $?\n";
                }
            }

            std::ofstream ofs;
            ofs.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
            ofs.open("dl-subs-" + stemFileName + ".sh", std::ios::out | std::ios::binary);
            ofs << srtScript;
            ofs.close();
        }
        else
        {
            r = EC_MODULE_UNKNOWN;

            cout << omw::fgBrightRed << "ERROR (" << "main.cpp:" << __LINE__ << ")" << endl;
        }


    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            util::printError("fatal error");
            cout << "    path1: " << ex.path1() << endl;
            cout << "    path2: " << ex.path2() << endl;
            cout << "    cat:   " << ex.code().category().name() << endl;
            cout << "    code:  " << ex.code().value() << endl;
            cout << "    msg:   " << ex.code().message() << endl;
            cout << "    what:  " << ex.what() << endl;
        }
    }
    catch (const std::system_error& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            util::printError("fatal error");
            cout << "    cat:   " << ex.code().category().name() << endl;
            cout << "    code:  " << ex.code().value() << endl;
            cout << "    msg:   " << ex.code().message() << endl;
            cout << "    what:  " << ex.what() << endl;
        }
    }
    catch (const std::exception& ex)
    {
        r = EC_ERROR;
        if (!quiet)
        {
            util::printError("fatal error");
            cout << "    what:  " << ex.what() << endl;
        }
    }
    catch (const int& ex)
    {
        if (r == EC_OK)
        {
            r = EC_ERROR;
            if (!quiet) util::printError("fatal error (" + std::to_string(ex) + ")");
        }
        else if (verbose) cout << "\n" << omw::fgBrightRed << "failed" << omw::defaultForeColor << endl;
    }
    catch (...)
    {
        if (r == EC_OK)
        {
            r = EC_ERROR;
            if (!quiet) util::printError("unspecified fatal error");
        }
        else if (verbose) cout << "\n" << omw::fgBrightRed << "failed" << omw::defaultForeColor << endl;
    }

    //if (r == EC_USER_ABORT) r = EC_OK;

    return r;
}
