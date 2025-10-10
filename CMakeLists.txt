cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 26)

project(Programming_Push_Back_Common)

# Need to: sudo apt-get install pkg-config  libusb-1.0-0-dev
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

add_executable(Programming_Push_Back_Common main.cpp
        utils.cpp
        SerialHandler.cpp
        Buffer.cpp
        main.cpp)

target_include_directories(Programming_Push_Back_Common PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(Programming_Push_Back_Common PRIVATE ${LIBUSB_LIBRARIES})
