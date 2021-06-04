cmake_minimum_required(VERSION 3.0)

project(huffman_encode VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-Os -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
-fdiagnostics-color=always -fdiagnostics-generate-patch \
-g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(root_code_path
    ../../../../../
)
set(player_path
    ${root_code_path}/stm32/stm32cubemx/f412g_player/src/
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${compile_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    ./
    ${player_path}/pictures
)

add_executable(huffman_encode
    huffman_encode.cpp
)

#target_link_libraries(huffman_encode
#    -static
#    -static-libstdc++    
#    -static-libgcc
#)


add_executable(offset_calculator
    offset_calculator.cpp
)

#target_link_libraries(offset_calculator
#    -static
#    -static-libstdc++    
#    -static-libgcc
#)

