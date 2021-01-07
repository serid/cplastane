cmake_minimum_required(VERSION 3.16)
project(cplastane)

set(CMAKE_CXX_STANDARD 14)

add_executable(cplastane
        main.cpp
        int.hxx
        strvec.hxx
        util.cxx
        util.hxx
        assembly/assembler.cxx
        assembly/assembler.hxx
        )

target_compile_options(cplastane PUBLIC -Wall -Wextra -Werror -Wno-missing-field-initializers)

set(CMAKE_C_FLAGS "-fsanitize=address -O0 -fno-omit-frame-pointer -g")
set(CMAKE_CXX_FLAGS "-fsanitize=address -O0 -fno-omit-frame-pointer -g")
set(CMAKE_LD_FLAGS "-fsanitize=address -O0 -fno-omit-frame-pointer -g")