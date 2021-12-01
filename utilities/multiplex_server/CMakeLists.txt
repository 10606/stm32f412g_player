cmake_minimum_required(VERSION 3.0)

project(multiplex_server VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-Os --pedantic -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20  -DNDEBUG --pedantic"
)

set(compile_flags 
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
    -fdiagnostics-color=always -fdiagnostics-generate-patch \
    --pedantic -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(root_code_path
    ../../../../../
)
set (player_path
    ${root_code_path}/stm32/stm32cubemx/f412g_player/src
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${release_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    src
    ${player_path}/main
    ${player_path}/view
    ${player_path}/usb
    ${player_path}/playlist
    ../packet_len_data
    ../ring_buffer
    ../sockets
)

add_executable(multiplex_server
    src/server.cpp
    src/client_wrapper.cpp
    src/com_wrapper.cpp
    src/epoll_wrapper.cpp
    src/unix_server_sock.cpp
    src/check_password.cpp
    src/tcp_server_sock.cpp
    ${player_path}/usb/usb_commands.cpp
    ../ring_buffer/ring_buffer.cpp
    ../sockets/web_socket.cpp
)

target_link_libraries(multiplex_server
    systemd
    crypto
)

