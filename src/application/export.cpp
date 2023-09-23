/*
author          Oliver Blaser
date            23.09.2023
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
#include "export.h"
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



int app::exprt(const app::Args& args, const app::Flags& flags, const std::string& m3uFile, const std::string& outDir)
{
    int r = EC_OK; // set to OK because of catch(...) and foreach inDirs

    IMPLEMENT_FLAGS();

    try
    {
        util::FileCounter fileCnt;
        util::ResultCounter rcnt = 0;
        omw::vector<std::string> postfixes;

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

        std::vector<omw::string> lines;
        // M3U util to be moved in another file
        {
            const omw::io::TxtFileInterface file(m3uFile);
            file.openRead();
            const size_t fileSize = file.size();
            omw::string text(fileSize, '\0');

            if (file.getState() != omw::io::TxtFileInterface::good) ERROR_PRINT_EC_THROWLINE("file not good before read", EC_ERROR);

            file.read(text.data(), fileSize);

            if(file.getState()!= omw::io::TxtFileInterface::good) ERROR_PRINT_EC_THROWLINE("file not good after read", EC_ERROR);

            

            if (text.size() >= 4)
            {
                if (text[0] == (char)(0x00) && text[1] == (char)(0x00) &&
                    text[2] == (char)(0xFe) && text[3] == (char)(0xFF))
                {
                    ERROR_PRINT_EC_THROWLINE("encoding not supported: UTF-32 BE", EC_ERROR);
                }
                if (text[0] == (char)(0xFF) && text[1] == (char)(0xFe) &&
                    text[2] == (char)(0x00) && text[3] == (char)(0x00))
                {
                    ERROR_PRINT_EC_THROWLINE("encoding not supported: UTF-32 LE", EC_ERROR);
                }
            }

            if (text.size() >= 2)
            {
                if (text[0] == (char)(0xFe) && text[1] == (char)(0xFF))
                {
                    ERROR_PRINT_EC_THROWLINE("encoding not supported: UTF-16 BE", EC_ERROR);
                }
                if (text[0] == (char)(0xFF) && text[1] == (char)(0xFe))
                {
                    ERROR_PRINT_EC_THROWLINE("encoding not supported: UTF-16 LE", EC_ERROR);
                }
            }

            if (text.size() >= 3)
            {
                if (text[0] == (char)(0xeF) && text[0] == (char)(0xBB) && text[0] == (char)(0xBF))
                {
                    if (verbose) util::printInfo("UTF8 BOM found");
                    text.erase(0, 3);
                }
            }

            const char* p = text.data();
            const char* end = text.data() + text.size();

            if (p < end) lines.assign(1, "");

            while (p < end)
            {
                const size_t nNewLine = omw::peekNewLine(p, end);

                if (nNewLine)
                {
                    lines.push_back("");
                    p += nNewLine;
                }
                else
                {
                    lines.back().push_back(*p);
                    ++p;
                }
            }

            if (lines.back().empty()) lines.pop_back();
        }

        for (size_t i = 0; i < lines.size(); ++i)
        {
            if ((lines[i].length() > 0) && (lines[i][0] == '#')) continue;

            fileCnt.addTotal();

#ifdef PRJ_DEBUG
            //cout << fileCnt.total() << " - " << lines[i] << endl;
#endif

#ifdef OMW_PLAT_WIN
            omw::string inFileStr = lines[i];

            constexpr size_t inFileStrReplPairsSize = 6;
            const omw::StringReplacePair inFileStrReplPairs_850[inFileStrReplPairsSize] = {
                omw::StringReplacePair(OMW_UTF8CP_Auml, "\x8E"),
                omw::StringReplacePair(OMW_UTF8CP_auml, "\x84"),
                omw::StringReplacePair(OMW_UTF8CP_Ouml, "\x99"),
                omw::StringReplacePair(OMW_UTF8CP_ouml, "\x94"),
                omw::StringReplacePair(OMW_UTF8CP_Uuml, "\x9A"),
                omw::StringReplacePair(OMW_UTF8CP_uuml, "\x81")
            };

            const omw::StringReplacePair inFileStrReplPairs_1252[inFileStrReplPairsSize] = { // Windows-1252 / ISO 8859-1
                omw::StringReplacePair(OMW_UTF8CP_Auml, "\xC4"),
                omw::StringReplacePair(OMW_UTF8CP_auml, "\xE4"),
                omw::StringReplacePair(OMW_UTF8CP_Ouml, "\xD6"),
                omw::StringReplacePair(OMW_UTF8CP_ouml, "\xF6"),
                omw::StringReplacePair(OMW_UTF8CP_Uuml, "\xDC"),
                omw::StringReplacePair(OMW_UTF8CP_uuml, "\xFC")
            };

            // TODO get system code page and convert properly

            inFileStr.replaceAll(inFileStrReplPairs_1252, inFileStrReplPairsSize);


            const fs::path inFile = inFileStr.c_str();
#else
            const fs::path inFile = lines[i].c_str();
#endif

            char tmpOutFileNumBuffer[20];
            sprintf(tmpOutFileNumBuffer, "%04zu", fileCnt.total());
            const fs::path outFile = outDir / fs::path(tmpOutFileNumBuffer + std::string("_") + inFile.filename().string());

            if (verbose) util::printFormattedLine("###copying \"" + inFile.u8string() + "\" to \"" + fs::weakly_canonical(outFile).u8string() + "\"");

            if (fs::exists(inFile))
            {
                if (fs::is_regular_file(inFile))
                {
                    try
                    {
                        fs::copy_file(inFile, outFile);
                        fileCnt.addCopied();
                    }
                    catch (const fs::filesystem_error& ex)
                    {
                        ERROR_PRINT(std::string("###") + ex.what());
                    }
                    // other exceptions will be catched by the main try-catch in this function
                }
                else ERROR_PRINT("###\"" + inFile.u8string() + "\" is not a file");
            }
            else ERROR_PRINT("###file \"" + inFile.u8string() + "\" not found");
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

    if (r == EC_USER_ABORT) r = EC_OK;

    return r;
}
