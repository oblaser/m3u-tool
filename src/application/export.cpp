/*
author          Oliver Blaser
date            05.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
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
#include <sstream>
#include <vector>

#include "application/cliarg.h"
#include "application/common.h"
#include "export.h"
#include "middleware/encoding-helper.h"
#include "middleware/util.h"
#include "project.h"

#include <omw/cli.h>
#include <omw/io/file.h>
#include <omw/string.h>
#include <omw/vector.h>
#include <omw/windows/windows.h>


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace
{
#ifdef PRJ_DEBUG
    const std::string magentaDebugStr = "\033[95mDEBUG\033[39m";
#endif
}



int app::exprt(const app::Args& args, const app::Flags& flags)
{
    int r = EC_ERROR;

    IMPLEMENT_FLAGS();

    util::CopyFileCounter fileCnt;
    util::ResultCounter rcnt = 0;

    // TODO make nicer
    const std::string m3uFileArg = args.raw.at(1);
    const std::string outDirArg = args.raw.at(2);

    const fs::path m3uFilePath = enc::path(m3uFileArg);
    const fs::path outDirPath = enc::path(outDirArg);

#if defined(PRJ_DEBUG) && 1
    app::dbg_rm_outDir(outDirPath);
#endif


    ///////////////////////////////////////////////////////////
    // check and read in file
    ///////////////////////////////////////////////////////////

    const m3u::M3U m3u = app::getFromUri(flags, util::Uri(m3uFileArg));


    ///////////////////////////////////////////////////////////
    // check/create out dir
    ///////////////////////////////////////////////////////////

    app::checkCreateOutDir(flags, outDirPath, outDirArg);


    ///////////////////////////////////////////////////////////
    // process
    ///////////////////////////////////////////////////////////

    const fs::path basePath = m3uFilePath.parent_path();

    for (size_t i = 0; i < m3u.entries().size(); ++i)
    {
        const auto& entry = m3u.entries().at(i);

        if (!entry.isResource()) continue;

        fileCnt.addTotal();

        fs::path inFilePath = enc::path(entry.data());
        if (!inFilePath.is_absolute()) inFilePath = basePath / inFilePath;

        std::stringstream filename;
        filename << std::setw(4) << std::setfill('0') << fileCnt.total();
        filename << '_' << inFilePath.filename().u8string();
        const fs::path outFilePath = outDirPath / enc::path(filename.str());

        if (verbose) app::printFormattedLine("###copying \"" + inFilePath.u8string() + "\" to \"" + fs::weakly_canonical(outFilePath).u8string() + "\"");

        if (fs::exists(inFilePath))
        {
            if (fs::is_regular_file(inFilePath))
            {
                try
                {
                    fs::copy_file(inFilePath, outFilePath);
                    fileCnt.addCopied();
                }
                catch (const fs::filesystem_error& ex)
                {
                    ERROR_PRINT(std::string("###") + ex.what());
                }
                // other exceptions will be catched by the main try-catch in this function
            }
            else ERROR_PRINT("###\"" + inFilePath.u8string() + "\" is not a file");
        }
        else ERROR_PRINT("###file \"" + inFilePath.u8string() + "\" not found");
    }


    ///////////////////////////////////////////////////////////
    // end
    ///////////////////////////////////////////////////////////

    if (!quiet)
    {
        cout << "========";
        
        cout << "  " << omw::fgBrightWhite;
        cout << fileCnt.copied() << "/" << fileCnt.total();
        cout << omw::normal << " exported";
            
        cout << ", ";
        if (rcnt.errors() != 0) cout << omw::fgBrightRed;
        cout << rcnt.errors();
        if (rcnt.errors() != 0) cout << omw::normal;
        cout << " error";
        if (rcnt.errors() != 1) cout << "s";
        
        cout << ", ";
        if (rcnt.warnings() != 0) cout << omw::fgBrightYellow;
        cout << rcnt.warnings();
        if (rcnt.warnings() != 0) cout << omw::normal;
        cout << " warning";
        if (rcnt.warnings() != 1) cout << "s";
        
        cout << "  ========" << endl;
    }
        
    if (((fileCnt.copied() == fileCnt.total()) && (rcnt.errors() != 0)) ||
        ((fileCnt.copied() != fileCnt.total()) && (rcnt.errors() == 0)))
    {
        r = EC_OK;
        throw (int)(__LINE__);
    }

    //if (verbose) cout << "\n" << omw::fgBrightGreen << "done" << omw::defaultForeColor << endl;
    
    if (fileCnt.copied() != fileCnt.total()) r = EC_ERROR;

    return r;
}
