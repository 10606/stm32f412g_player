cmake_minimum_required(VERSION 3.0)

project(onvert_font VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-O3 -flto -march=native -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(compile_flags "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
-fdiagnostics-color=always -fdiagnostics-generate-patch \
-g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

add_executable(convert_font
    src/convert_font.cpp
    ../characters/fill_char_set.cpp
    ../characters/utf8_automat.cpp
    src/psf/psf.cpp
    src/psf/psf_common.cpp
    src/psf/psf_1.cpp
    src/psf/psf_2.cpp
)

include_directories(
    src/
    src/psf/
    ../characters/
)

