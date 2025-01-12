/*
author          Oliver Blaser
date            17.12.2023
copyright       GPL-3.0 - Copyright (c) 2023 Oliver Blaser
*/

#ifndef IG_APP_CLIARG_H
#define IG_APP_CLIARG_H

#include <string>
#include <vector>

#include "project.h"

namespace argstr {

// add new args to
// - app::OptionList::checkOpt()
// - Args::containsXY() const
// - help text

const char* const force = "-f";
const char* const help = "-h";
const char* const help_alt = "--help";
const char* const noColor = "--no-color";
const char* const quiet = "-q";
const char* const verbose = "-v";
const char* const version = "--version";

} // namespace argstr

namespace app {

class FileList : public std::vector<std::string>
{
public:
    FileList() {}
    virtual ~FileList() {}

    virtual void add(const std::string& file) { push_back(file); }

    inline std::string getFile(size_t idx) const;

    bool isValid() const;
};

class OptionList : public std::vector<std::string>
{
public:
    OptionList();
    virtual ~OptionList() {}

    virtual void add(const std::string& opt);

    virtual bool contains(const std::string& arg) const;

    std::string unrecognized() const;

    bool isValid() const { return m_isValid; }

private:
    size_t m_unrecognizedIdx;
    bool m_isValid;

    void addOpt(const std::string& opt);
    bool checkOpt(const std::string& opt) const;
};

class Args
{
public:
    Args() {}
    Args(int argc, const char* const * argv) { parse(argc, argv); }
    Args(const std::vector<std::string>& args) { parse(args); }
    virtual ~Args() {}

    void parse(int argc, const char* const * argv);
    void parse(const std::vector<std::string>& args);
    void add(const std::string& arg);

    std::vector<std::string> inDirs() const;
    std::string outDir() const;

    OptionList& options() { return m_options; }
    const OptionList& options() const { return m_options; }

    // contains function in library base class
    bool contains(const std::string& option) const { return m_options.contains(option); }

    // contains functions in user derived cass
    bool containsForce() const { return m_options.contains(argstr::force); }
    bool containsHelp() const { return (m_options.contains(argstr::help) || m_options.contains(argstr::help_alt)); }
    bool containsNoColor() const { return m_options.contains(argstr::noColor); }
    bool containsQuiet() const { return m_options.contains(argstr::quiet); }
    bool containsVerbose() const { return m_options.contains(argstr::verbose); }
    bool containsVersion() const { return m_options.contains(argstr::version); }
    bool isGlobalHelp() const { return (!raw.empty() && ((raw[0] == argstr::help) || (raw[0] == argstr::help_alt))); }

    size_t count() const;
    size_t size() const;

    bool isValid() const;

    const std::string& operator[](size_t idx) const;

    std::vector<std::string> raw;
    bool isOption(size_t raw_idx) const; // hot fix, remove on the new nice and shiny args implementation

private:
    FileList m_files;
    OptionList m_options;
};

} // namespace app

#endif // IG_APP_CLIARG_H
