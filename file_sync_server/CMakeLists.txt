cmake_minimum_required(VERSION 3.20)
project(file_sync_server C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS ON)

add_executable(file_sync_server
        src/main.c
        src/read_data/merkle_tree.c
        src/read_data/acpt_data.c
        src/base/hld_err.c
        src/base/hld_err.h
        src/read_data/acpt_data.h
)

target_include_directories(file_sync_server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

