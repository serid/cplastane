cmake_minimum_required(VERSION 3.16)
project(cplastane)

set(CMAKE_CXX_STANDARD 20)

add_executable(cplastane
        main.cpp
        int.hxx
        strvec.hxx
        util.cxx
        util.hxx
        assembly/assembler.cxx
        assembly/assembler.hxx
        jit/jit.cxx
        jit/jit.hxx
        tests/assembly.cxx
        tests/assembly.hxx
        test/test.hxx
        os/alloc.cxx
        os/alloc.hxx
        )

target_compile_options(cplastane PUBLIC -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-variable -Wno-unused-function)

set(CMAKE_C_FLAGS "-fsanitize=address -O0 -fno-omit-frame-pointer -g")
set(CMAKE_CXX_FLAGS "-fsanitize=address -O0 -fno-omit-frame-pointer -g")
set(CMAKE_LD_FLAGS "-fsanitize=address -O0 -fno-omit-frame-pointer -g")