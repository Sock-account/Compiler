cmake_minimum_required(VERSION 4.2)
project(Compiler C)

set(CMAKE_C_STANDARD 11)

add_executable(Compiler
        main.c)

add_executable(T_test
        test.c)
