cmake_minimum_required(VERSION 3.22.1)
project(Programming_Push_Back_Common)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find relevent files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS *.hpp)
file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ./test/*.cc)

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

####################################################

# Get the GTEST framework
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(TestSuite  # Create test exec
    ${TEST_FILES}
)
target_link_libraries(  # Link tests with source files and test framework
    TestSuite
    Programming_Push_Back_Common_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(TestSuite DISCOVERY_MODE PRE_TEST)
