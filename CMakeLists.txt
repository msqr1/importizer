cmake_minimum_required(VERSION 3.31.5)
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
project(importizer VERSION 3.0.0)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()
if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
  message(FATAL_ERROR "Invalid build type '${CMAKE_BUILD_TYPE}', only Debug/Release are allowed")
endif()
if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(FATAL_ERROR "In-source build is unsupported")
endif()
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(BUILD_SHARED_LIBS OFF)
set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/3rd-party")
include("3rd-party/Cpm.cmake")

# Required by all targets
CPMAddPackage("gh:fmtlib/fmt#12.1.0")

# Common project configuration
add_library(common-config INTERFACE)
target_compile_features(common-config INTERFACE cxx_std_23)
target_compile_options(common-config INTERFACE
  -Wall
  -Werror
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti -fno-exceptions>
  $<$<CONFIG:Debug>:-O0 -g -fsanitize=undefined -fsanitize=address>
  $<$<CONFIG:Release>:-flto>
)
target_link_options(common-config INTERFACE
  $<$<CONFIG:Debug>:-O0 -fsanitize=undefined -fsanitize=address>
  $<$<CONFIG:Release>:-flto>
)
target_compile_definitions(common-config INTERFACE
  FMT_USE_EXCEPTIONS=0
)
target_include_directories(common-config INTERFACE "${CMAKE_SOURCE_DIR}/src")

add_subdirectory(src/importizer)
add_subdirectory(src/utils)
option(BUILD_TESTING "Setup and run tests" ON)
if(BUILD_TESTING)
  add_subdirectory(src/test)
  add_subdirectory(tests)
endif()
