cmake_minimum_required(VERSION 3.31.5)
set(CMAKE_OSX_DEPLOYMENT_TARGET 15)
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
set(BUILD_SHARED_LIBS OFF)
set(os $ENV{IMPORTIZER_OS})
set(root "${CMAKE_SOURCE_DIR}")


# 3rd-parties
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
set(3rdPartyDir "${root}/3rd-party")
set(CPM_SOURCE_CACHE "${3rdPartyDir}")
if(NOT EXISTS "${3rdPartyDir}/Cpm.cmake")
  file(DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.3/CPM.cmake
    "${3rdPartyDir}/Cpm.cmake"
    EXPECTED_HASH SHA256=a609e875fd532b067174250f6abbc3dac22fe2d64869783fb1e80bda1625c844
  )
endif()
include("${3rdPartyDir}/Cpm.cmake")
function(require target)
  foreach(pkg IN LISTS ARGN)
    if(pkg STREQUAL WindowsHModular)
      if(WIN32)
        CPMAddPackage("gh:Leandros/WindowsHModular#3adbb67f6f77025b5a9ef9f140c10e3393ca9093")

        # SYSTEM to supress warnings
        target_include_directories(${target} SYSTEM PRIVATE "${WindowsHModular_SOURCE_DIR}/include")
      endif()
    elseif(pkg STREQUAL tomlc17)
      # R260517
      CPMAddPackage("gh:cktan/tomlc17#cb9bba39f2e63a9e67fa61d6c7521a184eb5fc38")
      target_sources(${target} PRIVATE "${tomlc17_SOURCE_DIR}/src/tomlc17.c")

      # SYSTEM to supress warnings
      target_include_directories(${target} SYSTEM PRIVATE "${tomlc17_SOURCE_DIR}/src")

      if(WIN32)
        set_source_files_properties("${tomlc17_SOURCE_DIR}/src/tomlc17.c"
          PROPERTIES COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS
        )
      endif()
    elseif(pkg STREQUAL reproc)
      set(REPROC++ ON)

      # 14.2.7
      CPMAddPackage("gh:daandemeyer/reproc#06034a7fca1ec46eddb4997f7764db89380c5216")

      target_link_libraries(${target} PRIVATE reproc++)
      if(WIN32)
        target_compile_definitions(reproc PRIVATE _CRT_SECURE_NO_WARNINGS)
      endif()
    elseif(pkg STREQUAL fmt)
      # 12.1.0
      CPMAddPackage("gh:fmtlib/fmt#407c905e45ad75fc29bf0f9bb7c5c2fd3475976f")
      target_link_libraries(${target} PRIVATE fmt::fmt)
    elseif(pkg STREQUAL Clang)
      # Expecting static LLVM & Clang in 3rd-party
      set(Clang_DIR "${3rdPartyDir}/clang/lib/cmake/clang")
      set(LLVM_DIR "${3rdPartyDir}/llvm/lib/cmake/llvm")

      find_package(Clang REQUIRED)
      target_include_directories(${target} SYSTEM PRIVATE
        "${LLVM_INCLUDE_DIRS}"
        "${CLANG_INCLUDE_DIRS}"
      )
      target_link_libraries(${target} PRIVATE
        clangAST
        clangBasic
        clangTooling
        clangFrontend
        clangRewriteFrontend
        clangSerialization
      )
    else()
      message(FATAL_ERROR "Unknown 3rd-party library '${pkg}'")
    endif()
  endforeach()
endfunction()

# Project
## Common configuration
add_library(common-config INTERFACE)
target_compile_features(common-config INTERFACE cxx_std_23)
if(${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} STREQUAL GNU)
  target_compile_options(common-config INTERFACE
    -Wall
    -Werror
  )

  # Needs quoting and ';' for add_*_* but not target_*_*
  add_compile_options(
    "$<$<COMPILE_LANGUAGE:CXX>:
      -fno-rtti;
      -fno-exceptions
    >"
  )

  set(asan "-fsanitize=address")
elseif(MSVC)
  target_compile_options(common-config INTERFACE
    /W4
    /WX
  )

  # Since Clang was built with these settings, we must match them even in Debug mode
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
  add_compile_options(
    /utf-8
    "$<$<COMPILE_LANGUAGE:CXX>:
      /GR-;
      /EHs-c-
    >"
  )
  add_compile_definitions(
    "$<$<COMPILE_LANGUAGE:CXX>:_HAS_EXCEPTIONS=0;_ITERATOR_DEBUG_LEVEL=0>"
  )

  set(asan "/fsanitize=address")
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
set(ubsan "-fsanitize=undefined")
set(CMAKE_REQUIRED_LINK_OPTIONS ${ubsan})
check_cxx_compiler_flag(${ubsan} hasUbsan)
if(hasUbsan)
  add_compile_options($<$<CONFIG:Debug>:${ubsan}>)
  add_link_options($<$<CONFIG:Debug>:${ubsan}>)
endif()

## Still needed for dynamic linking to Windows CRT or MacOS libSystem
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(src/importizer)
add_subdirectory(src/utils)

option(BUILD_TESTING "Build test driver and run tests" ON)
if(BUILD_TESTING)
  add_subdirectory(src/run-test)
  add_subdirectory(tests)
endif()
