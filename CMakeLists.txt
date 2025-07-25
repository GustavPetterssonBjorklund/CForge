cmake_minimum_required(VERSION 3.15)

project(CForge)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.c)

add_executable(CForge ${SRC_FILES})
