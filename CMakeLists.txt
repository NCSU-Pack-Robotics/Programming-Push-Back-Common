cmake_minimum_required(VERSION 3.22.1)
set(CMAKE_CXX_STANDARD 23)

project(Push-Back-Common)

add_executable(Push-Back-Common COBS.hpp
        main.cpp
        SerialHandler.cpp
        SerialHandler.hpp
        COBS.cpp
        PacketId.hpp)