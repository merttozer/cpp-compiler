# CMake version requirement
cmake_minimum_required(VERSION 3.10)

# Project name
project(CppCompiler VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories (header files)
include_directories(include)

# Enable compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Add the executable
add_executable(cpp_compiler
    src/main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/IRGenerator.cpp
    src/Compiler.cpp
)

# Specify the output directory for the build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
