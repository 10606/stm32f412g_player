cmake_minimum_required(VERSION 3.0)

project(convert_playlist VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-O3 -mtune=native -march=native -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(compile_flags 
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
    -fdiagnostics-color=always -fdiagnostics-generate-patch \
    -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(root_code_path
    ../../../../../
)
set(FAT32_driver_path
    ${root_code_path}/FAT32_driver_char11/src
)
set (playlist_path
    ${root_code_path}/stm32/stm32cubemx/f412g_player/Src/playlist
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${compile_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    ${FAT32_driver_path}
    ${FAT32_driver_path}/lfn
    ${playlist_path}
)

link_directories(
    ${FAT32_driver_path}
)

add_library(libFAT
    ${FAT32_driver_path}/lfn/open_lfn.c
    ${FAT32_driver_path}/lfn/read_lfn_info.c
    ${FAT32_driver_path}/load_FAT/load_FAT.c
    ${FAT32_driver_path}/init/init.c
    ${FAT32_driver_path}/open/open.c
    ${FAT32_driver_path}/file_descriptor/file_descriptor.c
    ${FAT32_driver_path}/read/read.c
    ${FAT32_driver_path}/read_file_info/read_file_info.c
)

add_executable(convert_playlist
    convert_playlist.cpp 
)

target_link_libraries(convert_playlist
    libFAT
)

