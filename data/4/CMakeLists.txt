
project(wave_find_path)

cmake_minimum_required(VERSION 2.6)

set(SRCS
    wave_find_path.cpp
)

find_library(libSC "libSC" PATHS_SUFFIXES "../../lib")
find_library(libSCd "libSCd" PATHS_SUFFIXES "../../lib")

include_directories("../../include/libSC" "../../include/libTGF" "../../include/libSys")
add_executable(wave_find_path ${SRCS})
target_link_libraries(wave_find_path debug ${libSCd})
target_link_libraries(wave_find_path optimized ${libSC})
