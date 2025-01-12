/*
author          Oliver Blaser
date            17.12.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#include <string>
#include <utility>
#include <vector>

#include "cliarg.h"
#include "middleware/util.h"
#include "project.h"

#include <omw/intdef.h>
#include <omw/string.h>


namespace {}



inline std::string app::FileList::getFile(size_t idx) const { return (this->size() > idx ? this->at(idx) : ""); }

bool app::FileList::isValid() const { return (this->size() >= 2); }



app::OptionList::OptionList()
    : m_unrecognizedIdx(OMW_SIZE_MAX), m_isValid(true)
{}

void app::OptionList::add(const std::string& opt)
{
    if (opt.length() > 1)
    {
        if ((opt[1] != '-') && (opt.length() > 2))
        {
            for (size_t i = 1; i < opt.length(); ++i) { addOpt(std::string("-") + opt[i]); }
        }
        else addOpt(opt);
    }
    else addOpt(opt);
}

bool app::OptionList::contains(const std::string& arg) const
{
    for (const auto& e : *this)
    {
        if (e == arg) { return true; }
    }

    return false;
}

std::string app::OptionList::unrecognized() const { return (m_unrecognizedIdx != OMW_SIZE_MAX ? this->at(m_unrecognizedIdx) : ""); }

void app::OptionList::addOpt(const std::string& opt)
{
    if (!checkOpt(opt))
    {
        m_isValid = false;
        m_unrecognizedIdx = this->size();
    }

    this->push_back(opt);
}

bool app::OptionList::checkOpt(const std::string& opt) const
{
    return ((opt == argstr::force) || (opt == argstr::help) || (opt == argstr::help_alt) || (opt == argstr::noColor) || (opt == argstr::quiet) ||
            (opt == argstr::verbose) || (opt == argstr::version));
}



void app::Args::parse(int argc, const char* const * argv)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg(argv[i]);

        if (arg.length() > 0) add(arg);
    }
}

void app::Args::parse(const std::vector<std::string>& args)
{
    for (size_t i = 1; i < args.size(); ++i)
    {
        const auto& arg = args[i];

        if (arg.length() > 0) add(arg);
    }
}

void app::Args::add(const std::string& arg)
{
    if (arg[0] == '-') m_options.add(arg);
#ifdef OMW_PLAT_WIN
    else if (arg == "/?") m_options.add(argstr::help);
#endif
    else m_files.add(arg);

    raw.push_back(arg);
}

std::vector<std::string> app::Args::inDirs() const
{
    std::vector<std::string> r;

    const size_t n = m_files.size() - 1;

    if (n > 0) // needed because size is unsigned
    {
        for (size_t i = 0; i < n; ++i) { r.push_back(m_files[i]); }
    }

    return r;
}

std::string app::Args::outDir() const { return m_files.back(); }

size_t app::Args::count() const { return size(); }

size_t app::Args::size() const { return (m_files.size() + m_options.size()); }

bool app::Args::isValid() const
{
    return !raw.empty();

    // TODO the whole cliarg has to be redone anyway...

    // return (
    //     (m_files.isValid() && m_options.isValid()) ||
    //     (m_options.isValid() && (containsHelp() || containsVersion()))
    //     );
}

const std::string& app::Args::operator[](size_t idx) const
{
    if (idx < m_options.size()) return m_options[idx];
    else return m_files[idx - m_options.size()];
}

bool app::Args::isOption(size_t raw_idx) const
{
    bool r = false;

    const auto& arg = raw.at(raw_idx);

    if (arg.at(0) == '-') r = true;
#ifdef OMW_PLAT_WIN
    else if (arg == "/?") r = true;
#endif

    return r;
}
