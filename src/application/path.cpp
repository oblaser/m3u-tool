/*
author          Oliver Blaser
date            13.01.2024
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

    // TODO make nicer
    const std::string inFileArg = args.raw.at(1);
    const std::string outFileArg = args.raw.at(2);
    const std::string inBaseArg = ((args.raw.size() > 3)&&!args.isOption(3) ? args.raw.at(3) : "");
    const std::string outBaseArg = ((args.raw.size() > 4) && !args.isOption(4) ? args.raw.at(4) : "");

    const bool rmArg = args.contains("--remove");
    const bool checkExist = args.contains("--ck-exists");

    const fs::path outFilePath = enc::path(outFileArg);


    ///////////////////////////////////////////////////////////
    // check and read in file
    ///////////////////////////////////////////////////////////

    const m3u::M3U m3u = app::getFromUri(flags, util::Uri(inFileArg));


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
            std::string uri = e.data();

            if (uri.substr(0, inBaseArg.length()) == inBaseArg)
            {
                uri.erase(0, inBaseArg.length());

                omw::replaceAll(uri, '\\', '/');

                fs::path path;

                if (rmArg)
                {
                    if (inBaseArg.empty()) path = enc::path(uri).filename();
                    else if (uri.at(0) == '/') path = enc::path(uri.erase(0, 1));
                }
                else
                {
                    uri = outBaseArg + '/' + uri;
                    path = enc::path(uri);
                }

                target.add(m3u::Entry(path.lexically_normal().u8string(), e.ext()));
            }
            else
            {
                PRINT_WARNING("###INBASEPATH not found in entry " +
                    (e.hasExtension() ? (e.ext() + ' ') : std::string()) +
                    '"' + e.data() + '"');

                target.add(e);

                PRINT_INFO_V("entry is copied");
            }

            if (checkExist)
            {
                // TODO add file counter?

                const fs::path file ;
            }
        }
        else target.add(e);
    }

    util::writeFile(outFilePath, target.serialise());


    ///////////////////////////////////////////////////////////
    // end
    ///////////////////////////////////////////////////////////

    //if (verbose) cout << "\n" << omw::fgBrightGreen << "done" << omw::defaultForeColor << endl;

    return EC_OK;
}
