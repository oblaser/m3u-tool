#!/bin/bash

# author        Oliver Blaser
# date          27.01.2024
# copyright     GNU GPLv3 - Copyright (c) 2024 Oliver Blaser



source dep_globals.sh



errCnt=0
function procErrorCode()
{
    if [ $1 -ne 0 ]; then ((++errCnt)); fi;
}



ptintTitle "install ${prjName} to /opt/${prjDirName}/" 6

exitCode=0
{
    # TODO create pack_bin script and use that
    sudo mkdir -p "/opt/${prjDirName}" &&
    sudo cp "cmake/${prjBinName}" "/opt/${prjDirName}/" &&
    ptintTitle "done" 2
} ||
{
    exitCode=1
    ptintTitle "failed" 1
}

exit $exitCode
