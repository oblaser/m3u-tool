/*
author          Oliver Blaser
date            07.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "application/cliarg.h"
#include "application/common.h"
#include "application/processor.h"
#include "middleware/util.h"
#include "project.h"

#include <omw/cli.h>
#include <omw/defs.h>
#include <omw/windows/windows.h>


using std::cout;
using std::endl;
using std::setw;

namespace
{
    const std::string usageString = std::string(prj::exeName) + " MODULE ...";

    void printHelp()
    {
        constexpr int lw = 18;

        cout << prj::appName << endl;
        cout << endl;
        cout << "Usage:" << endl;
        cout << "  " << usageString << endl;
        cout << "  " << prj::exeName << " export INFILE OUTDIR [options]" << endl;
        cout << "  " << prj::exeName << " parse INFILE [options]" << endl;
        cout << "  " << prj::exeName << " path INFILE OUTFILE [INBASEPATH [OUTBASEPATH]] [options]" << endl;
        cout << "  " << prj::exeName << " vstreamdl INFILE OUTDIR NAME [MAX-RES-HEIGHT] [options]" << endl;
        cout << "  " << std::string(strlen(prj::exeName), ' ') << "     MAX-RES-HEIGHT defaults to 1080" << endl;
        cout << endl;
        cout << "Modules:" << endl;
        cout << std::left << setw(lw) << std::string("  ") + "export" << "copy and rename the files of the playlist" << endl;
        cout << std::left << setw(lw) << std::string("  ") + "parse" << "display the m3u entries" << endl;
        cout << std::left << setw(lw) << std::string("  ") + "vstreamdl" << "" << endl;
        cout << std::left << setw(lw) << std::string("  ") << omw::fgCyan << "tbd..." << omw::fgDefault << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << std::left << setw(lw) << std::string("  ") + argstr::force << "force overwriting output files" << endl;
        cout << std::left << setw(lw) << std::string("  ") + argstr::quiet << "quiet" << endl;
        cout << std::left << setw(lw) << std::string("  ") + argstr::verbose << "verbose" << endl;
        cout << std::left << setw(lw) << std::string("  ") + argstr::noColor << "monochrome console output" << endl;
        cout << std::left << setw(lw) << std::string("  ") + argstr::help + std::string(", ") + argstr::help_alt << "prints this help text" << endl;
        cout << std::left << setw(lw) << std::string("  ") + argstr::version << "prints version info" << endl;
        cout << std::left << setw(lw) << std::string("  ") << omw::fgCyan << "tbd..." << omw::fgDefault << endl;
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
        cout << "Copyright (c) 2024 Oliver Blaser." << endl;
        cout << "License: GNU GPLv3 <http://gnu.org/licenses/>." << endl;
        cout << "This is free software. There is NO WARRANTY." << endl;
    }
}



#ifdef OMW_PLAT_WIN
int wmain(int argc, wchar_t** argv)
{
#if defined(PRJ_DEBUG) && 0 // print argv
    for (int iarg = 0; iarg < argc; ++iarg)
    {
        bool utf = false;

        const size_t len = wcslen(argv[iarg]);
        for (size_t i = 0; i < len; ++i)
        {
            const auto& c = argv[iarg][i];

            if (c > 127)
            {
                if (!utf) cout << '[';
                cout << omw::toHexStr((uint16_t)c);
                utf = true;
            }
            else
            {
                if (utf) cout << ']';
                cout << (char)c;
                utf = false;
            }
        }

        cout << endl;
    }
#endif// print argv

    std::vector<std::string> rawArgs(argc);

    for (int i = 0; i < argc; ++i)
    {
        try { rawArgs[i] = omw::windows::wstou8(argv[i]); }
        catch (const std::exception& ex)
        {
            app::printError("faild to convert argv");
            app::printInfo(ex.what());
            std::wcout << L"argv[" << i << L"] " << argv[i] << endl; // console code page is not yet set to UTF-8
            return EC_ERROR;
        }
    }

#ifndef PRJ_DEBUG
    const
#endif // PRJ_DEBUG
    app::Args args(rawArgs);

#else // OMW_PLAT_WIN
int main(int argc, char** argv)
{
#ifndef PRJ_DEBUG
    const
#endif // PRJ_DEBUG
    app::Args args(argc, argv);
#endif // OMW_PLAT_WIN

    int r = 0;

#if defined(PRJ_DEBUG) && 1
    if (args.size() == 0)
    {
        //args.add("--help");

#if 0 // export
        
        args.add("export");

        //args.add("../../test/system/linux.m3u");
        args.add("../../test/system/f\xc3\xael\xc3\xab\xc3\xb1\xc3\xa0m\xc3\xa9.m3u");

        args.add("../../test/system/out-export-\xC5\xA4");

#elif 0 // parse m3u
        
        args.add("parse");
        
        args.add("../../test/system/ext-mixed.m3u");
        //args.add("../../test/system/f\xc3\xael\xc3\xab\xc3\xb1\xc3\xa0m\xc3\xa9.m3u");
        //args.add("../../test/system/hls.m3u");
        //args.add("../../test/system/non-ext.m3u");
        //args.add("https://raw.githubusercontent.com/oblaser/m3u-tool/main/test/system/hls.m3u");

#elif 1 // path

        args.add("path");

        // in file
        args.add("../../test/system/ext-mixed.m3u");
        //args.add("../../test/system/f\xc3\xael\xc3\xab\xc3\xb1\xc3\xa0m\xc3\xa9.m3u");

        // outfile
        args.add("../../test/system/out/path-out.m3u8");

        // in base
        //args.add("D:\\Musik\\Interpreten");
        //args.add("asdf");
        
        // out base
        //args.add("artists");

        args.add("--remove");
        args.add("-f");

#elif 1 // video stream download
        
        args.add("vstreamdl");
        
        //args.add("../../test/system/ext-mixed.m3u");
        //args.add("../../test/system/hls.m3u");
        args.add("https://raw.githubusercontent.com/oblaser/m3u-tool/main/test/system/hls.m3u?param=asdf&t=1230");
        //args.add("../../test/system/non-ext.m3u");
        
        args.add("../../test/system/out-vstreamdl-\xc3\xae");

        args.add("test-\xc3\xae");

        args.add("900");
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
#ifdef OMW_PLAT_WIN
    int dbg___getc_ = getc(stdin);
#endif
#endif

    cout << omw::normal << std::flush;

#ifdef OMW_PLAT_WIN
    winOutCodePageRes = omw::windows::consoleSetOutCodePage(winOutCodePage);
#endif

    return r;
}
