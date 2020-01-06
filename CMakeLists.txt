cmake_minimum_required(VERSION 3.15)
project(ex3_computer_structure C)

set(CMAKE_C_STANDARD 99)

add_executable(ex3_computer_structure cmake-build-debug/switch.c compile.c)