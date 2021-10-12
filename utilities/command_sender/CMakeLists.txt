cmake_minimum_required(VERSION 3.0)

project(command_sender VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-Os -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20 -DNDEBUG"
)

set(compile_flags 
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
    -fdiagnostics-color=always -fdiagnostics-generate-patch \
    -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(root_code_path
    ../../../../../
)
set(player_path
    ${root_code_path}/stm32/stm32cubemx/f412g_player/src/
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    ${player_path}/main
    ${player_path}/view
    ${player_path}/usb
    ${player_path}/playlist
    src/
    src/command_line/
    src/interactive/
    ../characters/
    ../multiplex_server/src/
)

add_executable(player_send_command
    src/commnad_sender.cpp
    src/command_line/send_by_name.cpp
    src/command_line/check_string.cpp
    src/interactive/interactive_fd.cpp
    src/interactive/term_display.cpp
    src/interactive/term_colors.cpp
    src/interactive/char_reconvert.cpp
    ../characters/fill_char_set.cpp
    ../characters/char_conversion.cpp
    ../multiplex_server/src/epoll_wrapper.cpp
)

target_link_libraries(player_send_command
    -static
    -static-libstdc++
    -static-libgcc
)

