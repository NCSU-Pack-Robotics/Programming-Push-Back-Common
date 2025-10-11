cmake_minimum_required(VERSION 3.22.1)
project(Programming_Push_Back_Common)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE test_files ./test/*.cc)
add_executable(
    TestSuite
    ${test_files}
)
target_link_libraries(
    TestSuite
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(TestSuite)

##########################

file(GLOB_RECURSE SRC_DIRS LIST_DIRECTORIES true *)
foreach(dir ${SRC_DIRS})
    if(IS_DIRECTORY ${dir})
        include_directories(${dir})
    endif()
endforeach()

file(GLOB_RECURSE SRC_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.hpp)
add_executable(Programming_Push_Back_Common
    ${SRC_FILES}
    ${HEADER_FILES}
)
