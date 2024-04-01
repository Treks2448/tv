cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang")

project(tv VERSION 1.0 LANGUAGES CXX)

# Compiler configuration
Set(CMAKE_CXX_STANDARD 23)
Set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# Add our code
add_executable(tv-host src/main.cpp)

# Add glfw libraries
add_subdirectory(lib/glfw)

# Link libraries
target_link_libraries(tv-host PUBLIC glfw)

# Set compiler flags for libraries
target_compile_options(glfw PRIVATE -O3)

# Set compiler flags for our code
target_compile_options(tv-host PRIVATE -O3 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)
