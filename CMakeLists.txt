cmake_minimum_required(VERSION 3.15)

project(CForge)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.c)

# Include json lib
include_directories(${CMAKE_SOURCE_DIR}/lib/json/single_include)

add_executable(CForge ${SRC_FILES})