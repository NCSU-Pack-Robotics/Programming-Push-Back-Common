cmake_minimum_required(VERSION 3.22.1)
project(Programming_Push_Back_Common)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TESTING "Include test code" OFF)

# Find relevent files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS *.hpp)

# Exclude build and external directories from source/header globs
list(FILTER SRC_FILES EXCLUDE REGEX ".*/cmake-build-.*|.*/_deps/.*|.*/CMakeFiles/.*")
list(FILTER HEADER_FILES EXCLUDE REGEX ".*/cmake-build-.*|.*/_deps/.*|.*/CMakeFiles/.*")

# Include all source directories
file(GLOB_RECURSE SRC_DIRS LIST_DIRECTORIES true *)
foreach(dir ${SRC_DIRS})
    if(IS_DIRECTORY ${dir})
        include_directories(${dir})
    endif()
endforeach()

# Build a static library from project sources so tests can link against it
add_library(Programming_Push_Back_Common_lib STATIC
    ${SRC_FILES}
    ${HEADER_FILES}
)

# When not testing (i.e., building for VEX Brain), apply ARM-specific flags
if (BRAIN AND NOT TESTING)
    target_compile_options(Programming_Push_Back_Common_lib PRIVATE
            -mcpu=cortex-a9
            -mfpu=neon-fp16
            -mfloat-abi=hard
            -Os
            -g
            -mthumb
            -ffunction-sections
            -fdata-sections
            -fdiagnostics-color
            -funwind-tables
            -D_POSIX_THREADS
            -D_UNIX98_THREAD_MUTEX_ATTRIBUTES
            -D_POSIX_TIMERS
            -D_POSIX_MONOTONIC_CLOCK
            -Wno-psabi
            -Wno-deprecated-declarations
    )
endif()

# This allows other projects using this library to reference anything in the src directory without needing a full reference
target_include_directories(Programming_Push_Back_Common_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (BRAIN)
    add_compile_definitions(BRAIN)
    # feels wrong to include dependencies specific to pros in common but idk how else to do this
    target_include_directories(Programming_Push_Back_Common_lib PUBLIC ../../include/)
endif()
if (PI)
    add_compile_definitions(PI)
    # Need to: sudo apt-get install pkg-config  libusb-1.0-0-dev
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

    # These links and includes need to be PUBLIC so that projects using this library also have access to the included libraries

    target_include_directories(Programming_Push_Back_Common_lib PUBLIC ${LIBUSB_INCLUDE_DIRS})
    target_link_libraries(Programming_Push_Back_Common_lib PUBLIC ${LIBUSB_LIBRARIES})
endif()

####################################################

if (TESTING)
    add_subdirectory(test/)
endif ()
