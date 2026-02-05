cmake_minimum_required(VERSION 3.24)

project(Can_damonium VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE configuration
add_subdirectory(JUCE)

# Common module for shared code
add_library(can_damonium_common INTERFACE)
target_include_directories(can_damonium_common INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
)

# IR Profiler Application (for recording standardized profiles)
add_subdirectory(src/profiler)

# VST3 Plugin
add_subdirectory(src/plugin)

# Standalone IR Recorder Application (for users)
add_subdirectory(src/recorder)
