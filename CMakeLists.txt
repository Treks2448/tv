cmake_minimum_required(VERSION 3.28.3)
project(tv VERSION 1.0 LANGUAGES CXX)

Set(CMAKE_CXX_STANDARD 23)

Set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(lib/glfw)

add_executable(tv-host src/main.cpp)
target_link_libraries(tv-host glfw)

