/*
author          Oliver Blaser
date            12.11.2023
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
    int r = EC_ERROR;

    const auto flags = app::Flags(args.containsForce(),
                args.containsQuiet(),
                args.containsVerbose());

    IMPLEMENT_FLAGS();






    if (args.raw.at(0) == "export")
    {
        r = app::exprt(args, flags);
    }
    else if (args.raw.at(0) == "parse")
    {
        auto dbg = m3u::M3U(args.raw.at(2));

        for (const auto& e : dbg.entries())
        {
            if (e.isRegularRes()) cout << omw::fgBrightCyan << "R " << omw::fgDefault << e.path() << endl;
            else if (e.isComment()) cout << omw::fgBrightBlack << "C " << omw::fgDefault << e.data() << endl;
            else if (e.isExtension()) cout << omw::fgGreen << "X " << omw::fgDefault << e.ext() << endl;
            else if (e.hasExtension()) cout << omw::fgBrightYellow << "X " << omw::fgDefault << e.ext() << " \"" << omw::fgBrightWhite << e.path() << omw::fgDefault << "\"" << endl;
            else cout << omw::fgBrightRed << "E " << omw::fgDefault << e.path() << endl;
        }

        cout << "\n===================================================\n" << dbg.serialize() << "<EOF>==============================================" << endl;
    }
    else
    {
        r = -1;

        cout << omw::fgBrightRed << "ERROR (" << "main.cpp:" << __LINE__ << ")" << endl;
    }





    return r;
}
