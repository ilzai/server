cmake_minimum_required(VERSION 3.10)
project(client)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(client client.cpp)
target_link_libraries(client ${GTEST_LIBRARIES} pthread)