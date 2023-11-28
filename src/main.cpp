/*
author          Oliver Blaser
date            12.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "application/cliarg.h"
#include "application/processor.h"
#include "project.h"

#include <omw/cli.h>
#include <omw/windows/windows.h>


using std::cout;
using std::endl;
using std::setw;

namespace
{
    const std::string usageString = std::string(prj::exeName) + " module INPUT_M3U_FILE ...";

    void printHelp()
    {
        constexpr int lw = 18;

        cout << prj::appName << endl;
        cout << endl;
        cout << "Usage:" << endl;
        cout << "  " << usageString << endl;
        cout << endl;
        cout << "Modules:" << endl;
        cout << std::left << setw(lw) << std::string("  ") << "tbd..." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << std::left << setw(lw) << std::string("  ") << "tbd..." << endl;
        //cout << std::left << setw(lw) << std::string("  ") + argstr::force << "force overwriting output files" << endl;
        //cout << std::left << setw(lw) << std::string("  ") + argstr::quiet << "quiet" << endl;
        //cout << std::left << setw(lw) << std::string("  ") + argstr::verbose << "verbose" << endl;
        //cout << std::left << setw(lw) << std::string("  ") + argstr::noColor << "monochrome console output" << endl;
        //cout << std::left << setw(lw) << std::string("  ") + argstr::help + std::string(", ") + argstr::help_alt << "prints this help text" << endl;
        //cout << std::left << setw(lw) << std::string("  ") + argstr::version << "prints version info" << endl;
        cout << endl;
        cout << "Website: <" << prj::website << ">" << endl;
    }

    void printUsageAndTryHelp()
    {
        cout << "Usage: " << usageString << "\n\n";
        cout << "Try '" << prj::exeName << " --help' for more options." << endl;
    }

    void printVersion()
    {
        const omw::Version& v = prj::version;

        cout << prj::appName << "   ";
        if (v.isPreRelease()) cout << omw::fgBrightMagenta;
        cout << v.toString();
        if (v.isPreRelease()) cout << omw::defaultForeColor;
#ifdef PRJ_DEBUG
        cout << "   " << omw::fgBrightRed << "DEBUG" << omw::defaultForeColor << "   " << __DATE__ << " " << __TIME__;
#endif
        cout << endl;

        cout << endl;
        cout << "project page: " << prj::website << endl;
        cout << endl;
        cout << "Copyright (c) 2023 Oliver Blaser." << endl;
        cout << "License: GNU GPLv3 <http://gnu.org/licenses/>." << endl;
        cout << "This is free software. There is NO WARRANTY." << endl;
    }
}



int main(int argc, char** argv)
{
    int r = 0;

#ifdef PRJ_DEBUG
    app::Args args(argc, argv);
#else
    const app::Args args(argc, argv);
#endif

#if defined(PRJ_DEBUG) && 1
    if (args.size() == 0)
    {
#if 0 // export
        
        args.add("export");

        args.add("../../../test/system/pl.m3u");

        //args.add("../../../test/system/pl");
        args.add("../../../test/system/out-export/");

#elif 1 // parse m3u
        
        args.add("parse");
        
        //args.add("../../../test/system/ext-mixed.m3u");
        //args.add("../../../test/system/hls.m3u");
        //args.add("../../../test/system/non-ext.m3u");
        args.add("https://raw.githubusercontent.com/oblaser/m3u-tool/main/test/system/hls.m3u");

#elif 1 // video stream download
        
        args.add("vstreamdl");
        
        //args.add("../../../test/system/hls.m3u");
        args.add("https://raw.githubusercontent.com/oblaser/m3u-tool/main/test/system/hls.m3u");
        
        args.add("../../../test/system/out-vstreamdl");
#else
//#warning "nop"
#endif
        // options
        //args.add("-vf");
        args.add("-v");
        //args.add("-f");
        //args.add("-h");
        //args.add("-q");
        //args.add("--version");
    }
#endif

    if (args.containsNoColor()) omw::ansiesc::disable();
    else
    {
        bool envt = true;
#ifdef OMW_PLAT_WIN
        envt = omw::windows::consoleEnVirtualTermProc();
#endif
        omw::ansiesc::enable(envt);
    }

#ifdef OMW_PLAT_WIN
    const auto winOutCodePage = omw::windows::consoleGetOutCodePage();
    bool winOutCodePageRes = omw::windows::consoleSetOutCodePage(65001);
#endif

#ifndef PRJ_DEBUG
    if (prj::version.isPreRelease()) cout << omw::fgBrightMagenta << "pre-release v" << prj::version.toString() << omw::defaultForeColor << endl;
#endif

#if defined(PRJ_DEBUG) && 1
    cout << omw::foreColor(26) << "--======# args #======--\n";
    for (size_t i = 0; i < args.size(); ++i) cout << " " << args[i] << endl;
    cout << "--======# end args #======--" << endl << omw::defaultForeColor;
#endif

    if (args.isValid())
    {
        if (/*args.containsHelp()*/ args.isGlobalHelp()) printHelp();
        else if (args.containsVersion()) printVersion();
        else r = app::process(args);
    }
    else
    {
        r = 1;

        if (args.count() == 0)
        {
            cout << "No arguments." << endl;
            printUsageAndTryHelp();
        }
        else if (!args.options().isValid())
        {
            cout << prj::exeName << ": unrecognized option: '" << args.options().unrecognized() << "'" << endl;
            printUsageAndTryHelp();
        }
        else
        {
            cout << "Error" << endl;
            printUsageAndTryHelp();
        }
    }

#if defined(PRJ_DEBUG) && 1
    cout << omw::foreColor(26) << "===============\nreturn " << r << "\npress enter..." << omw::normal << endl;
    int dbg___getc_ = getc(stdin);
#endif

    cout << omw::normal << std::flush;

#ifdef OMW_PLAT_WIN
    winOutCodePageRes = omw::windows::consoleSetOutCodePage(winOutCodePage);
#endif

    return r;
}
