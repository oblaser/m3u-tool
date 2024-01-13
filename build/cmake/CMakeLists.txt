
# author        Oliver Blaser
# date          05.01.2024
# copyright     GNU GPLv3 - Copyright (c) 2024 Oliver Blaser

cmake_minimum_required(VERSION 3.13)

project(m3u-tool)

set(BINNAME m3u-tool)



if(_DEBUG)
    add_compile_definitions(_DEBUG)
endif(_DEBUG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)



include_directories(../../src/)

set(SOURCES
../../src/application/cliarg.cpp
../../src/application/common.cpp
../../src/application/export.cpp
../../src/application/path.cpp
../../src/application/processor.cpp
../../src/application/vstreamdl.cpp
../../src/middleware/curl-helper.cpp
../../src/middleware/encoding-helper.cpp
../../src/middleware/m3u.cpp
../../src/middleware/util.cpp
../../src/main.cpp
)



include_directories(../../sdk/omw/include)
link_directories(../../sdk/omw/lib)



add_executable(${BINNAME} ${SOURCES})
target_link_libraries(${BINNAME} libomw.a curl)
