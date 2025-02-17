/*
author          Oliver Blaser
date            20.01.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
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

#include "application/common.h"
#include "middleware/encoding-helper.h"
#include "middleware/util.h"
#include "project.h"
#include "vstreamdl.h"

#include <omw/cli.h>
#include <omw/io/file.h>
#include <omw/string.h>
#include <omw/vector.h>


using std::cout;
using std::endl;

namespace fs = std::filesystem;

namespace {

// if url is a relative path, prepend url from m3u
std::string handleUrl(const std::string& url, const util::Uri& m3uFileUri)
{
    std::string r = url;

    auto streamUri = util::Uri(url);

    if (streamUri.scheme().empty() && streamUri.authority().empty() && // is relative path
        !m3uFileUri.scheme().empty() && !m3uFileUri.authority().empty())
    {
        const fs::path basePath = enc::path(m3uFileUri.path()).parent_path();
        const std::string path = basePath.u8string() + '/' + streamUri.path();

        streamUri.setScheme(m3uFileUri.scheme());
        streamUri.setAuthority(m3uFileUri.authority());
        streamUri.setPath(path);

        r = streamUri.string();
    }

    return r;
}

} // namespace



int app::vstreamdl(const app::Args& args, const app::Flags& flags)
{
    int r = EC_ERROR;

    IMPLEMENT_FLAGS();

    MessageCounter msgCnt = 0;
    util::ResultCounter rcnt = 0;

    // TODO make nicer
    const std::string m3uFileArg = args.raw.at(1);
    const std::string outDirArg = args.raw.at(2);
    const std::string outNameArg = args.raw.at(3);
    const std::string maxResHArg = ((args.raw.size() > 4) && !args.isOption(4) ? args.raw.at(4) : "1080");

    const bool noSubsArg = args.contains("--no-subs");
    const bool saveOrigArg = args.contains("--save-original");

    const util::Uri m3uFileUri = util::Uri(m3uFileArg);
    const fs::path outDirPath = enc::path(outDirArg);

    if (!omw::isUInteger(maxResHArg)) ERROR_PRINT_EC_THROWLINE("invalid MAX-RES-HEIGHT", EC_ERROR);
    const int maxResHeight = std::stoi(maxResHArg);

#if defined(PRJ_DEBUG) && 0
    app::dbg_rm_outDir(outDirPath);
#endif


    ///////////////////////////////////////////////////////////
    // check and read in file
    ///////////////////////////////////////////////////////////

    const m3u::HLS hls = getFromUri(msgCnt, flags, m3uFileUri);


    ///////////////////////////////////////////////////////////
    // check/create out dir
    ///////////////////////////////////////////////////////////

    app::checkCreateOutDir(msgCnt, flags, outDirPath, outDirArg);


    ///////////////////////////////////////////////////////////
    // process
    ///////////////////////////////////////////////////////////

    if (saveOrigArg && m3uFileUri.isUrl())
    {
        const std::string origFileName = outNameArg + ".original.m3u8";
        const fs::path origFilePath = outDirPath / enc::path(origFileName);

        checkOutFile(msgCnt, flags, origFilePath, origFileName, "original file");

        util::writeFile(origFilePath, hls.serialise());
        PRINT_INFO_V("created file \"" + fs::weakly_canonical(origFilePath).u8string() + "\"");
    }

    if (!omw::isUInteger(maxResHArg)) ERROR_PRINT_EC_THROWLINE("invalid MAX-RES-HEIGHT", EC_ERROR);

    const bool hasAudio = !hls.audioStreams().empty();
    const bool hasVideo = !hls.streams().empty();
    const bool hasSubtitles = !hls.subtitles().empty();

    if (!hasAudio && !hasVideo && !hasSubtitles) ERROR_PRINT_EC_THROWLINE("empty stream", EC_STREAM_EMPTY);

    if (!hasAudio && hasVideo) WARNING_PRINT("no audio");
    if (hasAudio && !hasVideo) WARNING_PRINT("no video");
    if (hasAudio || hasVideo)
    {
        std::string txt = "";

#if 0
        for (size_t i = 0; i < hls.otherEntries().size(); ++i)
        {
            txt += hls.otherEntries()[i].serialise();
            txt += m3u::serialiseEndOfLine;
        }
#else
        txt += m3u::extm3u_str;
        txt += m3u::serialiseEndOfLine;
#endif

        txt += m3u::serialiseEndOfLine;

        for (size_t i = 0; i < hls.audioStreams().size(); ++i)
        {
            auto astream = hls.audioStreams()[i];

            astream.setUri(::handleUrl(astream.uri(), m3uFileUri));

            txt += astream.serialise();
            txt += m3u::serialiseEndOfLine;
        }

        txt += m3u::serialiseEndOfLine;

        size_t streamIdx = 0;
        for (size_t i = 1; i < hls.streams().size(); ++i)
        {
            const int h = hls.streams()[i].resolutionHeight();

            if ((h > hls.streams()[streamIdx].resolutionHeight()) && (h <= maxResHeight)) streamIdx = i;
        }
        auto vstream = hls.streams()[streamIdx];

        // no stream found with resolution <= maxResHeight
        if (vstream.resolutionHeight() > maxResHeight)
        {
            // TODO improve with force and verbosity flags
            WARNING_PRINT("stream resolution: " + vstream.resolutionExtParam().value().data());
        }

        vstream.setData(::handleUrl(vstream.data(), m3uFileUri));

        txt += vstream.serialise();
        txt += m3u::serialiseEndOfLine;

        const std::string outFileName = outNameArg + ".m3u8";
        const fs::path outFilePath = outDirPath / enc::path(outFileName);

        checkOutFile(msgCnt, flags, outFilePath, outFileName, "output file");

        util::writeFile(outFilePath, txt);
        PRINT_INFO_V("created file \"" + fs::weakly_canonical(outFilePath).u8string() + "\"");
    }
    else WARNING_PRINT("no audio and no video");

    if (!noSubsArg)
    {
        if (hasSubtitles)
        {
            std::string srtScript = "";
            srtScript += "# generated with " + std::string(prj::appName) + " " + std::string(prj::website) + "\n";
            srtScript += "# generated at " + util::getDateTimeStr() + "\n";
            srtScript += "\n";
            srtScript += "mkdir -p subs\n";
            srtScript += "\n";

            for (const auto& st : hls.subtitles())
            {
                if (!st.uri().empty())
                {
                    const std::string filename = "./subs/" + outNameArg + "-" + st.language() + (st.forced() ? "-forced" : "") + ".srt";

                    srtScript += "ffmpeg -i \"" + ::handleUrl(st.uri(), m3uFileUri) + "\" -scodec srt -loglevel warning \"" + filename + "\"\n";
                    srtScript += "echo $?\n";
                }
            }

            const std::string scriptFileName = "dl-subs-" + outNameArg + ".sh";
            const fs::path scriptFilePath = outDirPath / enc::path(scriptFileName);

            checkOutFile(msgCnt, flags, scriptFilePath, scriptFileName, "subs dl script");

            util::writeFile(scriptFilePath, srtScript);
            PRINT_INFO_V("created file \"" + fs::weakly_canonical(scriptFilePath).u8string() + "\"");
        }
        else if (verbose) INFO_PRINT("no subtitles");
    }


    ///////////////////////////////////////////////////////////
    // end
    ///////////////////////////////////////////////////////////

    // if (verbose) cout << "\n" << omw::fgBrightGreen << "done" << omw::defaultForeColor << endl;
    if (rcnt.errors() == 0) { r = EC_OK; }

    return r;
}
