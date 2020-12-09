cmake_minimum_required(VERSION 3.0)

project(command_sender VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-O3 -mtune=native -march=native -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(compile_flags 
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
    -fdiagnostics-color=always -fdiagnostics-generate-patch \
    -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(root_code_path
    ../../../../
)
set (usb_command_path
    ${root_code_path}/stm32/stm32cubemx/f412g_player/Src/usb
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${compile_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    ${usb_command_path}
)

add_executable(player_send_command
    commnad_sender.cpp
    send_by_name.cpp
    check_string.cpp
    #interactive.cpp
    interactive_fd.cpp
    headers.cpp
)

