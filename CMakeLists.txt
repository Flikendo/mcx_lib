cmake_minimum_required(VERSION 3.10)

project(mcx_lib VERSION 1.0 LANGUAGES CXX)

add_executable(mcx_lib main.cpp)

target_compile_features(mcx_lib PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(mcx_lib PRIVATE /W4)
else()
    target_compile_options(mcx_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()