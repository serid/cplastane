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