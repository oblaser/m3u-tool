/*
author          Oliver Blaser
date            09.05.2023
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

#include "export.h"
#include "middleware/util.h"
#include "project.h"

#include <omw/cli.h>
#include <omw/io/file.h>
#include <omw/string.h>
#include <omw/vector.h>
#include <omw/windows/string.h>


#define IMPLEMENT_FLAGS()           \
const bool quiet = flags.quiet;     \
bool ___verbose = flags.verbose;    \
const bool& verbose = ___verbose;   \
if (quiet) ___verbose = false;

#define ERROR_PRINT(msg)            \
{                                   \
    rcnt.incErrors();               \
    if (!quiet) printError(msg);    \
}

#define INFO_PRINT(msg)         \
{                               \
    if (!quiet) printInfo(msg); \
}

#define WARNING_PRINT(msg)          \
{                                   \
    rcnt.incWarnings();             \
    if (!quiet) printWarning(msg);  \
}

#define ERROR_PRINT_EC_THROWLINE(msg, EC_x) \
{                                   \
    rcnt.incErrors();               \
    if (!quiet) printError(msg);    \
    r = EC_x;                       \
    throw (int)(__LINE__);          \
}


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace
{
    enum ERRORCODE // https://tldp.org/LDP/abs/html/exitcodes.html / on MSW are no preserved codes
    {
        EC_OK = 0,
        EC_ERROR = 1,

        EC__begin_ = 79,

        EC_OUTDIR_NOTEMPTY = EC__begin_,
        EC_INOUTDIR_EQ, // TODO implment
        EC_OUTDIR_NOTCREATED,
        EC_M3UFILE_NOT_FOUND,

        EC_USER_ABORT, // not actually returned, used internally

        EC__end_,

        EC__max_ = 113
    };
    static_assert(EC__end_ <= EC__max_, "too many error codes defined");

    // 
    // "### normal "quoted bright" white"
    // "### normal @just bright@ white"
    // 
    void printFormattedText(const std::string& text)
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

    void printFormattedLine(const std::string& text)
    {
        printFormattedText(text);
        cout << endl;
    }

    constexpr int ewiWidth = 10;
    void printError(const std::string& text)
    {
        cout << omw::fgBrightRed << std::left << std::setw(ewiWidth) << "error:" << omw::defaultForeColor;
        printFormattedText(text);
        cout << endl;
    }
    void printInfo()
    {
        cout << omw::fgBrightCyan << std::left << std::setw(ewiWidth) << "info:" << omw::defaultForeColor;
    }
    void printInfo(const std::string& text)
    {
        printInfo();
        printFormattedText(text);
        cout << endl;
    }
    void printWarning(const std::string& text)
    {
        cout << omw::fgBrightYellow << std::left << std::setw(ewiWidth) << "warning:" << omw::defaultForeColor;
        printFormattedText(text);
        cout << endl;
    }

    void printTitle(const std::string& title)
    {
        //cout << omw::fgBrightWhite << title << omw::normal << endl;
        cout << title << endl;
    }



#pragma region library
    int cliChoice(const std::string& q, int def = 0, char first = 'y', char second = 'n')
    {
        int r = 0;
        const omw::string a(1, first);
        const omw::string b(1, second);
        omw::string data;

        do
        {
            std::cout << q << " [" << (def == 1 ? a.toUpper_ascii() : a) << "/" << (def == 2 ? b.toUpper_ascii() : b) << "] ";
            std::getline(std::cin, data);

            if (data.toLower_ascii() == a) r = 1;
            else if (data.toLower_ascii() == b) r = 2;
            else if (data.length() == 0) r = def;
            else r = 0;
        }
        while ((r != 1) && (r != 2));

        return r;
    }

    omw::string to_string(uint64_t val, int base, const char* digits)
    {
        omw::string r = "";

        if (val == 0) r += digits[0];

        while (val != 0)
        {
            r = digits[val % base] + r; // use reverse() instead
            val /= base;
        }

        return r;
    }
#pragma endregion


    omw::string getDirName(const fs::path& dir)
    {
        omw::string r;
        
        if (dir.has_filename()) r = dir.filename().u8string();
        else r = dir.parent_path().filename().u8string();

        return r;
    }



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



int exprt::process(const std::string& m3uFile, const std::string& outDir, const app::Flags& flags)
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
                        printInfo(msg);

                        if (2 == cliChoice("use non empty OUTDIR?"))
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
                    if (verbose) printInfo("UTF8 BOM found");
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

            std::string inFileStr = lines[i];

            const fs::path inFile = inFileStr;

            if (fs::exists(inFile))
            {
                if (fs::is_regular_file(inFile))
                {
                    char tmp[20];
                    sprintf(tmp, "%04zu", fileCnt.total());

                    const fs::path outFile = outDir / fs::path(tmp + std::string(" - ") + inFile.filename().string());

                    cout << inFile << " ==> " << outFile << endl;

                    fileCnt.addCopied();

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
            printError("fatal error");
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
            printError("fatal error");
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
            printError("fatal error");
            cout << "    what:  " << ex.what() << endl;
        }
    }
    catch (const int& ex)
    {
        if (r == EC_OK)
        {
            r = EC_ERROR;
            if (!quiet) printError("fatal error (" + std::to_string(ex) + ")");
        }
        else if (verbose) cout << "\n" << omw::fgBrightRed << "failed" << omw::defaultForeColor << endl;
    }
    catch (...)
    {
        if (r == EC_OK)
        {
            r = EC_ERROR;
            if (!quiet) printError("unspecified fatal error");
        }
        else if (verbose) cout << "\n" << omw::fgBrightRed << "failed" << omw::defaultForeColor << endl;
    }

    if (r == EC_USER_ABORT) r = EC_OK;

    return r;
}
