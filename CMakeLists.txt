cmake_minimum_required(VERSION 3.15)

project(CForge)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Use FetchContent to download dependencies
include(FetchContent)

# Fetch SFML
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# Fetch nlohmann_json
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(nlohmann_json)

# Source files
file(GLOB_RECURSE ASSEMBLER_SRC_FILES src/*.cpp src/*.c)
file(GLOB_RECURSE EMULATOR_SRC_FILES emulator/*.cpp emulator/*.c)

# Create executables
add_executable(CForge ${ASSEMBLER_SRC_FILES})
add_executable(CForgeEmulator ${EMULATOR_SRC_FILES})

# Set C++17 for both targets
target_compile_features(CForge PRIVATE cxx_std_17)
target_compile_features(CForgeEmulator PRIVATE cxx_std_17)

# Link libraries
target_link_libraries(CForge PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(CForgeEmulator PRIVATE 
    SFML::Graphics 
    SFML::Window 
    SFML::System
    nlohmann_json::nlohmann_json
)