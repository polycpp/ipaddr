cmake_minimum_required(VERSION 3.20)
project(polycpp_ipaddr VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library target (header-only with a single compilation unit)
add_library(polycpp_ipaddr
    src/ipaddr.cpp
)
target_include_directories(polycpp_ipaddr PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Alias for consistent naming
add_library(polycpp::ipaddr ALIAS polycpp_ipaddr)

# Tests
option(POLYCPP_IPADDR_BUILD_TESTS "Build tests" ON)
if(POLYCPP_IPADDR_BUILD_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY git@github.com:google/googletest.git
        GIT_TAG        v1.15.2
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(test_ipaddr tests/test_ipaddr.cpp)
    target_link_libraries(test_ipaddr PRIVATE polycpp_ipaddr GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(test_ipaddr)
endif()
