cmake_minimum_required(VERSION 3.31)
project(importizer VERSION 3.0.0)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
if(NOT (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES))
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()
if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST CMAKE_CONFIGURATION_TYPES)
  message(FATAL_ERROR "Unknown build type '${CMAKE_BUILD_TYPE}', only Debug/Release is allowed")
endif()
if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(FATAL_ERROR "In-source build is unsupported")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(root "${CMAKE_SOURCE_DIR}")
set(3rdPartyDir "${root}/3rd-party")
set(scriptsDir "${root}/scripts")

# Project
add_library(common-config INTERFACE)
target_compile_features(common-config INTERFACE cxx_std_23)
if(${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} STREQUAL GNU)
  target_compile_options(common-config INTERFACE
    -Wall
    -Werror
  )
  add_compile_options(
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti;-fno-exceptions>"
  )

  set(asan -fsanitize=address)
elseif(MSVC)
  target_compile_options(common-config INTERFACE
    /W4
    /WX
  )

  # Since Clang was built with these settings, we must match them even in Debug mode
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
  add_compile_options(
    /utf-8
    "$<$<COMPILE_LANGUAGE:CXX>:/GR-;/EHs-c->"
  )
  add_compile_definitions(
    "$<$<COMPILE_LANGUAGE:CXX>:_HAS_EXCEPTIONS=0;_ITERATOR_DEBUG_LEVEL=0>"
  )

  set(asan /fsanitize=address)
endif()
target_include_directories(common-config INTERFACE "${CMAKE_SOURCE_DIR}/src")
if(POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
endif()
include(CheckIPOSupported)
check_ipo_supported(RESULT res)
if(res)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()
include(CheckCXXCompilerFlag)
set(CMAKE_REQUIRED_LINK_OPTIONS ${asan})
check_cxx_compiler_flag(${asan} hasAsan)
if(hasAsan)
  add_compile_options($<$<CONFIG:Debug>:${asan}>)
  add_link_options($<$<CONFIG:Debug>:${asan}>)
endif()
set(ubsan -fsanitize=undefined)
set(CMAKE_REQUIRED_LINK_OPTIONS ${ubsan})
check_cxx_compiler_flag(${ubsan} hasUbsan)
if(hasUbsan)
  add_compile_options($<$<CONFIG:Debug>:${ubsan}>)
  add_link_options($<$<CONFIG:Debug>:${ubsan}>)
endif()

option(TRACE_BUILD "Enable build tracing for use with something like ClangBuildAnalyzer" OFF)
if(TRACE_BUILD)
  add_compile_options(
    -ftime-trace
    -ftime-trace-granularity=1
  )
endif()

# 3rd-party stuff
include(scripts/3rdParty.cmake)

add_subdirectory(src/importizer)
add_subdirectory(src/utils)

option(BUILD_TESTING "Build test driver & run tests" ON)
if(BUILD_TESTING)
  add_subdirectory(src/run-test)
  add_subdirectory(tests)
endif()
