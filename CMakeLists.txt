cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 26)

project(Programming_Push_Back_Common)

add_executable(Programming_Push_Back_Common main.cpp
        utils.cpp
        SerialHandler.cpp
        Buffer.cpp)