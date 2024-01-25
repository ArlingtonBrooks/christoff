# CMAKE Header
cmake_minimum_required(VERSION 3.5)
project(CMake_Tut)

####
# This statement forces a build type of "Release" by default
# (you have to specify Debug type release otherwise)
####
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

####
# The executable is the thing that will produce an executable binary
####
add_executable(Christoff Christoff.cpp)
target_compile_options(Christoff PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(Christoff ${CURSES_LIBRARIES})

