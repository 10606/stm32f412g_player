cmake_minimum_required(VERSION 3.0)

project(command_sender_tcp VERSION 1.0 LANGUAGES C CXX)

set(release_flags
    "-Os -flto -march=native -Wall -ftemplate-depth=10000 -std=c++20  -DNDEBUG"
)

set(compile_flags 
    "-fsanitize=address -fsanitize=leak -fsanitize=undefined \
    -fdiagnostics-color=always -fdiagnostics-generate-patch \
    -g -Wall -ftemplate-depth=10000 -std=c++20"
)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${compile_flags}")

message("CMAKE_CXX_FLAGS" ${CMAKE_CXX_FLAGS})

include_directories(
    ../multiplex_server/src
    ../command_sender/src
    ../packet_len_data
)

add_executable(command_sender_tcp 
    src/send_password.cpp
    ../packet_len_data/recver_data.cpp
    ../packet_len_data/sender_data.cpp
    ../multiplex_server/src/epoll_reg.cpp
    ../multiplex_server/src/epoll_wrapper.cpp
)

target_link_libraries(command_sender_tcp
    crypto
)

