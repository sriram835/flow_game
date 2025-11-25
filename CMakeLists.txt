cmake_minimum_required(VERSION 3.16)

project(MyRaylibGame LANGUAGES CXX)

# Use at least C++17 (change if you want)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Let CMake find the raylib package installed via pacman
find_package(raylib REQUIRED)  # uses /usr/lib/cmake/raylib/raylib-config.cmake on Arch[web:22]

# Collect all source files under src/
file(GLOB_RECURSE PROJECT_SOURCES
     "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Include your header directory
include_directories("${CMAKE_SOURCE_DIR}/header")

# Create the executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# Link with raylib
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
