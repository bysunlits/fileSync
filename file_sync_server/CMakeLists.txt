cmake_minimum_required(VERSION 3.20)
project(file_sync_server)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS ON)

add_executable(file_sync_server
        src/main.c
        src/read_data/merkle_tree.c
        src/read_data/merkle_tree.h
        src/read_data/acpt_data.c
        src/read_data/acpt_data.h
        src/base/hld_err.c
        src/base/hld_err.h
        src/base/logger.c
        libs/logger/logger.h
        src/utils/log_sinker.c
        src/utils/log_sinker.h
        src/read_data/sock_connect.c
        src/read_data/sock_connect.h
        include/epoll/epoll.c
        include/epoll/epoll.h
)

target_include_directories(file_sync_server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/logger
)

