cmake_minimum_required(VERSION 3.0)

project(convert_playlist VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-Os --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20  -DNDEBUG"
)

set(compile_flags 
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
    -fdiagnostics-color=always -fdiagnostics-generate-patch \
    --pedantic -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(root_code_path
    ../../../../../
)
set(FAT32_driver_path
    ${root_code_path}/FAT32_driver_char11/src
)
set (player_path
    ${root_code_path}/stm32/stm32cubemx/f412g_player/src
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${compile_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    ${FAT32_driver_path}
    ${FAT32_driver_path}/lfn
    ${player_path}/playlist
    ${player_path}/main
    ../characters
)

link_directories(
    ${FAT32_driver_path}
)

add_library(libFAT
    ${FAT32_driver_path}/lfn/open_lfn.cpp
    ${FAT32_driver_path}/lfn/read_lfn_info.cpp
    ${FAT32_driver_path}/fat_info/fat_info.cpp
    ${FAT32_driver_path}/fat_info/load_FAT.cpp
    ${FAT32_driver_path}/open/open.cpp
    ${FAT32_driver_path}/file_descriptor/read.cpp
    ${FAT32_driver_path}/read_file_info/read_file_info.cpp
)

add_executable(convert_playlist
    src/convert_playlist.cpp 
    src/path_conversion.cpp
    ../characters/char_conversion.cpp
    ../characters/fill_char_set.cpp
    ../characters/convert_custom.cpp
)

target_link_libraries(convert_playlist
    # -static
    libFAT
    # -static-libstdc++    
    # -static-libgcc
)

