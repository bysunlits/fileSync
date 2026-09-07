cmake_minimum_required(VERSION 4.3)
project(server C)

set(CMAKE_C_STANDARD 17)

include_directories(include)

add_executable(server
        src/main.c)
