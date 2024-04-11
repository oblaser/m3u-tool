/*
author          Oliver Blaser
date            27.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "application/common.h"
#include "middleware/encoding-helper.h"
#include "middleware/util.h"
#include "path.h"
#include "project.h"

#include <omw/cli.h>


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace
{
}



int app::path(const app::Args& args, const app::Flags& flags)
{
    IMPLEMENT_FLAGS();

    MessageCounter msgCnt = 0;
    util::ExistsFileCounter fileCnt;

    // TODO make nicer
    const std::string inFileArg = args.raw.at(1);
    const std::string outFileArg = args.raw.at(2);
    const std::string inBaseArg = ((args.raw.size() > 3) && !args.isOption(3) ? args.raw.at(3) : "");
    const std::string outBaseArg = ((args.raw.size() > 4) && !args.isOption(4) ? args.raw.at(4) : "");

    const bool rmArg = args.contains("--remove");
    const bool checkExistArg = args.contains("--ck-exists");

    const fs::path outFilePath = enc::path(outFileArg);


    ///////////////////////////////////////////////////////////
    // check and read in file
    ///////////////////////////////////////////////////////////

    const m3u::M3U m3u = app::getFromUri(msgCnt, flags, util::Uri(inFileArg));


    ///////////////////////////////////////////////////////////
    // check out file path
    ///////////////////////////////////////////////////////////

    bool outFileExtIsU8 = true;
    if (outFilePath.extension() != ".m3u8")
    {
        outFileExtIsU8 = false;

        PRINT_WARNING_V("OUTFILE is not using m3u8 extension");
        PRINT_INFO_V("OUTFILE encoding will be UTF-8 (without BOM) anyway");
    }

    if (fs::exists(outFilePath))
    {
        if (flags.force) { PRINT_WARNING_V("overwriting OUTFILE"); }
        else
        {
            const std::string msg = "###OUTFILE \"" + outFileArg + "\" already exists";

            if (verbose)
            {
                app::printInfo(msg);

                if (2 == omw_::cli::choice("overwrite OUTFILE?"))
                {
                    throw app::processor_exit(EC_USER_ABORT);
                }
            }
            else PRINT_ERROR_EXIT(msg, EC_OUTFILE_EXISTS);
        }
    }

    if (!fs::exists(outFilePath.parent_path()))
    {
        PRINT_ERROR_EXIT("###could not find \"" + outFilePath.parent_path().u8string() + "\"", EC_OUTDIR_NOTFOUND);
    }


    ///////////////////////////////////////////////////////////
    // process
    ///////////////////////////////////////////////////////////

    m3u::M3U target;
    size_t entryIndex = 0;

    const auto headerEntry = m3u::Entry("", m3u::extm3u_str);
    const auto extencEntry = m3u::Entry("", m3u::extenc_str + std::string("UTF-8"));

    if ((m3u.entries().size() > 1) &&
        (m3u.entries().at(0) == headerEntry) &&
        m3u.entries().at(1).extIs(m3u::extenc_str))
    {
        if ((m3u.entries().at(1).extParam().size() < 1) || (omw_::toUpper(m3u.entries().at(1).extParam().at(0).value()) != "UTF-8"))
        {
            PRINT_ERROR_EXIT("###encoding not supported: @" + m3u.entries().at(1).extParam().at(0).value().data() + "@", EC_ERROR);
        }
    }

    if (!outFileExtIsU8 && !m3u.isEmpty() && (m3u.entries().at(0) == headerEntry))
    {
        target.add(headerEntry);
        target.add(extencEntry);

        if ((m3u.entries().size() > 1) && m3u.entries().at(1).extIs(m3u::extenc_str)) entryIndex = 2;
        else entryIndex = 1;
    }

    if (outFileExtIsU8 && (m3u.entries().size() > 1) &&
        (m3u.entries().at(0) == headerEntry) &&
        m3u.entries().at(1).extIs(m3u::extenc_str))
    {
        target.add(headerEntry);

        entryIndex = 2;
    }

    for (size_t i = entryIndex; i < m3u.entries().size(); ++i)
    {
        const auto& e = m3u.entries()[i];

        if (e.isResource())
        {
            m3u::Entry newEntry = e;

            std::string uriStr = e.data();

            if (uriStr.substr(0, inBaseArg.length()) == inBaseArg)
            {
                uriStr.erase(0, inBaseArg.length());

                omw::replaceAll(uriStr, '\\', '/');

                fs::path path;

                if (rmArg)
                {
                    if (inBaseArg.empty()) path = enc::path(uriStr).filename();
                    else if (uriStr.at(0) == '/') path = enc::path(uriStr.erase(0, 1));
                    else path = enc::path(uriStr);
                }
                else path = enc::path(outBaseArg + '/' + uriStr);

                newEntry = m3u::Entry(path.lexically_normal().u8string(), e.ext());
            }
            else
            {
                PRINT_WARNING("###INBASEPATH not found in entry " +
                    (e.hasExtension() ? (e.ext() + ' ') : std::string()) +
                    '"' + e.data() + '"');

                // e is already assigned to newEntry

                PRINT_INFO_V("entry is copied");
            }

            target.add(newEntry);

            if (checkExistArg)
            {
                const fs::path file = enc::path(newEntry.data());
                const fs::path fileRelToOutFile = outFilePath.parent_path() / file;

#ifdef PRJ_DEBUG
                const auto wd = fs::current_path();
                const auto absWd = fs::absolute(wd);
                const auto absFile = fs::absolute(file);
                const auto absRel = fs::absolute(fileRelToOutFile);
                const auto absRelWd = fs::absolute(wd / file);
#endif // PRJ_DEBUG

                if (fs::exists(file) || fs::exists(fileRelToOutFile))
                {
                    fileCnt.addExists();
                }
                else PRINT_WARNING_V("###output file \"" + file.u8string() + "\" not found");
            }

            fileCnt.addTotal();
        }
        else target.add(e);
    }

    util::writeFile(outFilePath, target.serialise());


    ///////////////////////////////////////////////////////////
    // end
    ///////////////////////////////////////////////////////////

    int r = EC_OK;

    if (checkExistArg)
    {
        constexpr size_t threshold = 7;

        if ((verbose && (msgCnt > threshold) && (fileCnt.exists() != fileCnt.total())) ||
            (verbose && (msgCnt > threshold)))
        {
            cout << "========";

            cout << "  " << omw::fgBrightWhite;
            cout << fileCnt.exists() << "/" << fileCnt.total();
            cout << omw::normal << " files found";

            cout << "  ========" << endl;
        }
    }

    return r;
}
