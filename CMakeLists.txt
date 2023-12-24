cmake_minimum_required(VERSION 3.12)

project(labwork6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(labwork6 main.cpp
        Parser.cpp Parser.h
        Archive.cpp Archive.h
        HammingCode.cpp HammingCode.h)