cmake_minimum_required(VERSION 3.15)
project(betterr VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# The library
add_library(betterr src/betterr.c)

target_include_directories(betterr
    PUBLIC include
)

# The tests
add_executable(test_betterr tests/test_betterr.c)

target_link_libraries(test_betterr PRIVATE betterr)