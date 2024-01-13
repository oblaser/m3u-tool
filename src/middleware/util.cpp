/*
author          Oliver Blaser
date            12.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "util.h"

#include <omw/omw.h>


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace
{
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

omw::string util::getDirName(const fs::path& dir)
{
    omw::string r;

    if (dir.has_filename()) r = dir.filename().u8string();
    else r = dir.parent_path().filename().u8string();

    return r;
}

std::string util::readFile(const std::filesystem::path& file)
{
    std::stringstream txt;

    std::ifstream ifs;
    ifs.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
    ifs.open(file, std::ios::in | std::ios::binary); // binary so that no conversations happen
    txt << ifs.rdbuf();
    ifs.close();

    return txt.str();
}

void util::writeFile(const fs::path& file, const std::string& text)
{
    std::ofstream ofs;
    ofs.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
    ofs.open(file, std::ios::out | std::ios::binary); // binary so that nothing gets converted
    ofs << text;
    ofs.close();
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

std::string omw_::toUpper(const std::string& str)
{
    std::string r = str;

    for (auto& c : r)
    {
        if ((c >= 'a') && (c <= 'z')) c -= 32;
    }

    return r;
}
