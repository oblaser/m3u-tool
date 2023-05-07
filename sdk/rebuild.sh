#!/bin/bash

# author        Oliver Blaser
# date          07.05.2023
# copyright     GPL-3.0 - Copyright (c) 2023 Oliver Blaser

cd ./omw/

cd ./lib/ &&
{
    rm -f *.a
    rm -f *.so*
    cd ../
}

cd ./build/
./make_clean.sh
./cmake_clean.sh
cd ../../



./build.sh
