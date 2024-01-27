#!/bin/bash

# author        Oliver Blaser
# date          11.11.2023
# copyright     GNU GPLv3 - Copyright (c) 2023 Oliver Blaser

prjName="m3u-tool"
prjDisplayName="M3U Tool"
prjBinName=$prjName
prjDirName=$prjName
repoDirName=$prjName
copyrightYear="2024"

versionstr=$(head -n 1 dep_vstr.txt)

function ptintTitle()
{
    if [ "$2" = "" ]
    then
        echo "  --=====#   $1   #=====--"
    else
        echo -e "\033[3$2m  --=====#   \033[9$2m$1\033[3$2m   #=====--\033[39m"
    fi
}

# pass output filename as argument
function writeReadmeFile()
{
    echo ""                                 > $1
    echo "${prjDisplayName} v${versionstr}" >> $1
    echo ""                                 >> $1
    echo "GNU GPLv3 - Copyright (c) ${copyrightYear} Oliver Blaser" >> $1
    echo ""                                 >> $1
    echo "https://github.com/oblaser/${repoDirName}" >> $1
}
