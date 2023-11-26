/*
author          Oliver Blaser
date            26.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "defines.h"
#include "m3u-helper.h"
#include "middleware/util.h"
#include "project.h"
#include "vstreamdl.h"

#include <omw/cli.h>
#include <omw/io/file.h>
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



#if defined(PRJ_DEBUG)
    void dbg_rm_outDir(const std::string& outDir)
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
}



int app::vstreamdl(const app::Args& args, const app::Flags& flags)
{
    int r = EC_OK; // set to OK because of catch(...) in processor.cpp

    IMPLEMENT_FLAGS();

    // TODO make nicer
    const std::string m3uFile = args.raw.at(1);
    const std::string outDir = args.raw.at(2);

    util::ResultCounter rcnt = 0;

    const fs::path m3uFilePath = m3uFile;
    const fs::path outDirPath = outDir;

#if defined(PRJ_DEBUG) && 1
    dbg_rm_outDir(outDir);
#endif


    ///////////////////////////////////////////////////////////
    // check in file
    ///////////////////////////////////////////////////////////

    if (!fs::exists(m3uFile)) ERROR_PRINT_EC_THROWLINE("M3U file not found", EC_M3UFILE_NOT_FOUND);


    ///////////////////////////////////////////////////////////
    // check/create out dir
    ///////////////////////////////////////////////////////////

    if (fs::exists(outDir))
    {
        if (!fs::is_empty(outDir))
        {
            if (flags.force)
            {
                if (verbose) WARNING_PRINT("using non empty OUTDIR");
            }
            else
            {
                const std::string msg = "###OUTDIR \"" + outDir + "\" is not empty";

                if (verbose)
                {
                    util::printInfo(msg);

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

        if (!fs::exists(outDir)) ERROR_PRINT_EC_THROWLINE("failed to create OUTDIR", EC_OUTDIR_NOTCREATED);
    }


    ///////////////////////////////////////////////////////////
    // process
    ///////////////////////////////////////////////////////////

    const m3u::HLS hls = getFromUri(r, flags, m3uFile);

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

    const auto scriptFile = outDirPath / ("dl-subs-" + stemFileName + ".sh");

    std::ofstream ofs;
    ofs.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
    ofs.open(scriptFile, std::ios::out | std::ios::binary);
    ofs << srtScript;
    ofs.close();


    ///////////////////////////////////////////////////////////
    // end
    ///////////////////////////////////////////////////////////

    //if (verbose) cout << "\n" << omw::fgBrightGreen << "done" << omw::defaultForeColor << endl;
    r = EC_OK;

    return r;
}
