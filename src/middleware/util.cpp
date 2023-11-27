/*
author          Oliver Blaser
date            27.11.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "util.h"

#include <omw/cli.h>
#include <omw/omw.h>


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace
{
    constexpr int ewiWidth = 10;
}



std::string util::getDateTimeStr(time_t t, const char* strftimeFormat)
{
    constexpr size_t bufferSize = 100;
    char buffer[bufferSize];
    std::string r;

#if defined(OMW_PLAT_WIN)
    std::tm dateTime;
    const std::tm* pDateTime = nullptr;
    const auto error = localtime_s(&dateTime, &t);
    if (error == 0) pDateTime = &dateTime;
#else
    const struct std::tm* pDateTime = std::localtime(&t);
#endif

    if (pDateTime && (std::strftime(buffer, bufferSize, strftimeFormat, pDateTime) > 0)) r = buffer;
    else r = "[" + std::to_string(t) + "]";

    return r;
}

// 
// "### normal "quoted bright" white"
// "### normal @just bright@ white"
// 
void util::printFormattedText(const std::string& text)
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

void util::printFormattedLine(const std::string& text)
{
    printFormattedText(text);
    cout << endl;
}

void util::printError(const std::string& text)
{
    cout << omw::fgBrightRed << std::left << std::setw(ewiWidth) << "error:" << omw::defaultForeColor;
    printFormattedText(text);
    cout << endl;
}

void util::printInfo()
{
    cout << omw::fgBrightCyan << std::left << std::setw(ewiWidth) << "info:" << omw::defaultForeColor;
}

void util::printInfo(const std::string& text)
{
    printInfo();
    printFormattedText(text);
    cout << endl;
}

void util::printWarning(const std::string& text)
{
    cout << omw::fgBrightYellow << std::left << std::setw(ewiWidth) << "warning:" << omw::defaultForeColor;
    printFormattedText(text);
    cout << endl;
}

void util::printTitle(const std::string& title)
{
    //cout << omw::fgBrightWhite << title << omw::normal << endl;
    cout << title << endl;
}

omw::string util::getDirName(const fs::path& dir)
{
    omw::string r;

    if (dir.has_filename()) r = dir.filename().u8string();
    else r = dir.parent_path().filename().u8string();

    return r;
}



int omw_::cli::choice(const std::string& q, int def, char first, char second)
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

omw::string omw_::to_string(uint64_t val, int base, const char* digits)
{
    omw::string r = "";

    if (val == 0) r += digits[0];

    while (val != 0)
    {
        r = digits[val % base] + r; // maybe use reverse() instead
        val /= base;
    }

    return r;
}
