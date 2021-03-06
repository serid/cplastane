cmake_minimum_required(VERSION 3.16)
project(cplastane)

set(CMAKE_CXX_STANDARD 20)

add_executable(cplastane
        main.cpp
        int.hxx
        strvec.hxx
        util/util.cxx
        util/util.hxx
        assembly/assembly.cxx
        assembly/assembly.hxx
        jit/jit.cxx
        jit/jit.hxx
        tests/assembly.cxx
        tests/assembly.hxx
        test/test.hxx
        os/alloc.cxx
        os/alloc.hxx
        util/option/option.hxx
        assembly/parse/parse.cxx
        assembly/parse/parse.hxx
        parsec/parsec.cxx
        parsec/parsec.hxx
        test/test.cxx
        parsec/tests/tests.cxx
        parsec/tests/tests.hxx
        util/result/result.hxx
        )

target_compile_options(cplastane PUBLIC -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-variable -Wno-unused-function)

set(CMAKE_C_FLAGS "-O0 -fno-omit-frame-pointer -g")
set(CMAKE_CXX_FLAGS "-O0 -fno-omit-frame-pointer -g")
set(CMAKE_LD_FLAGS "-O0 -fno-omit-frame-pointer -g")