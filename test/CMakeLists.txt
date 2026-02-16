cmake_minimum_required(VERSION 3.22.1)
project(Programming_Push_Back_Common_Tests)

set(CMAKE_CXX_STANDARD 23)

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
        PacketTest.cc
        CobsTest.cc
)

target_link_libraries(  # Link tests with source files and test framework
        TestSuite
        Programming_Push_Back_Common_lib
        GTest::gtest_main
        ${LIBUSB_LIBRARIES}
)

target_include_directories(TestSuite PRIVATE ${LIBUSB_INCLUDE_DIRS})

include(GoogleTest)
gtest_discover_tests(TestSuite DISCOVERY_MODE PRE_TEST)