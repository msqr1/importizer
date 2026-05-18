# Lowest CMake on Github Actions that supports C++ modules
cmake_minimum_required(VERSION 3.31.5)
project(importizer VERSION 2.0.0)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(FATAL_ERROR "In-source build is unsupported")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CPM_SOURCE_CACHE "3rdParty")
include("3rdParty/cpm.cmake")
CPMAddPackage("gh:fmtlib/fmt#12.1.0")
CPMAddPackage("gh:cktan/tomlc17#R260517")

find_package(Clang REQUIRED)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
  message(FATAL_ERROR "Invalid build type '${CMAKE_BUILD_TYPE}', only Debug/Release are allowed")
endif()

add_subdirectory(src)
option(TESTS "Setup and run tests" OFF)
if(TESTS)
  add_subdirectory(tests)
endif()
