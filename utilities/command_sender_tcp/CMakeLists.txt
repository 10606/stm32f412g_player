cmake_minimum_required(VERSION 3.0)

project(command_sender_tcp VERSION 1.0 LANGUAGES C CXX)

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
    ../multiplex_server/src
    ../command_sender/src
    ../packet_len_data
    ../command_sender/src/interactive
    ../characters/
    ../sockets/
)

add_executable(command_sender_tcp 
    src/send_password.cpp
    ../multiplex_server/src/epoll_wrapper.cpp
    ../command_sender/src/interactive/char_reconvert.cpp
    ../command_sender/src/interactive/interactive_fd.cpp
    ../command_sender/src/interactive/term_colors.cpp
    ../command_sender/src/interactive/term_display.cpp
    ../characters/fill_char_set.cpp
    ../characters/char_conversion.cpp
    ../characters/convert_custom.cpp
    ../characters/calc_utf8_len.cpp
)

target_link_libraries(command_sender_tcp
    crypto
)

