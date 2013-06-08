cmake_minimum_required(VERSION 2.8)

project(RefactoringPuzzle CXX)

enable_testing()

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -stdlib=libc++")
endif()

set(GTEST_HOME gtest)

add_subdirectory(${GTEST_HOME})
include_directories(${GTEST_HOME}/include)

include_directories(include)

add_subdirectory(include)
add_subdirectory(test)
