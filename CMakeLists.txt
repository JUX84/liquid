cmake_minimum_required(VERSION 2.6.4)

project(Liquid)
set(CMAKE_MODULE_PATH cmake)

INCLUDE(CheckIncludeFiles)
INCLUDE(FindPkgConfig)
INCLUDE(CheckLibraryExists)

FIND_PACKAGE(LibEV REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
link_directories(${LibEV_LIBRARY_DIRS})
include_directories(${LibEV_INCLUDE_DIRS})

file(GLOB_RECURSE source_files src/*)
include_directories(inc)

set(EXECUTABLE_OUTPUT_PATH bin)
add_executable(liquid ${source_files})

target_link_libraries(liquid ev z)

set(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
