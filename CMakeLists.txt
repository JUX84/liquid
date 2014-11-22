cmake_minimum_required(VERSION 2.6.4)

project(Liquid)

set(CMAKE_MODULE_PATH cmake)

INCLUDE(CheckIncludeFiles)
INCLUDE(FindPkgConfig)
INCLUDE(CheckLibraryExists)

FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(LibEV REQUIRED)
link_directories(${LibEV_LIBRARY_DIRS} ${ZLIB_LIBRARY_DIRS})
include_directories(${LibEV_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

include_directories("/usr/local/include/mariadb/")
include_directories("/usr/include/mariadb/")
link_directories("/usr/lib/x86_64-linux-gnu") #debian
link_directories("/usr/local/lib/mariadb") #debian

file(GLOB_RECURSE source_files src/*)
include_directories(inc)

set(EXECUTABLE_OUTPUT_PATH bin)
add_executable(liquid ${source_files})

target_link_libraries(liquid ev z)
target_link_libraries(liquid mysqlclient)

option(USE_CLANG "build with clang" ON)

set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -O2")
if(USE_CLANG)
	set(CMAKE_CXX_COMPILER "clang++")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	target_link_libraries(liquid c++abi)
endif(USE_CLANG)
