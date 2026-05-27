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

# Common project configuration
add_library(common-config INTERFACE)

set(3rdPartyDir "${CMAKE_SOURCE_DIR}/3rd-party")
set(CPM_SOURCE_CACHE "${3rdPartyDir}")
include("${3rdPartyDir}/Cpm.cmake")
function(require target)
  foreach(pkg IN LISTS ARGN)
    if(${pkg} STREQUAL "WindowsHModular" AND WIN32)
      CPMAddPackage("gh:Leandros/WindowsHModular#master")

      # SYSTEM to supress warnings
      target_include_directories("${target}" SYSTEM PRIVATE
        "${WindowsHModular_SOURCE_DIR}/include")
    elseif(${pkg} STREQUAL tomlc17)
      CPMAddPackage("gh:cktan/tomlc17#R260517")
      target_sources("${target}" PRIVATE "${tomlc17_SOURCE_DIR}/src/tomlc17.c")

      # SYSTEM to supress warnings
      target_include_directories("${target}" SYSTEM PRIVATE "${tomlc17_SOURCE_DIR}/src")

      if(WIN32)
        # Supress secure CRT warnings for tomlc17
        set_source_files_properties("${tomlc17_SOURCE_DIR}/src/tomlc17.c" PROPERTIES
          COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS")

        # Native MSVC
        if(MSVC AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
          set_source_files_properties("${tomlc17_SOURCE_DIR}/src/tomlc17.c" PROPERTIES
            COMPILE_OPTIONS "/W0")
        endif()
      endif()
    elseif(${pkg} STREQUAL subprocess.h)
      CPMAddPackage("gh:sheredom/subprocess.h#main")
      target_include_directories("${target}" PRIVATE "${subprocess.h_SOURCE_DIR}")
    elseif(${pkg} STREQUAL Clang)
      if(WIN32)
        # Expecting static LLVM build in 3rd-party/llvm
        set(Clang_DIR "${3rdPartyDir}/llvm/lib/cmake/clang")
      endif()
      find_package(Clang REQUIRED)

      # SYSTEM to supress warnings
      target_include_directories("${target}" SYSTEM PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
      )

      target_link_libraries("${target}" PRIVATE
        clangAST
        clangBasic
        clangTooling
        clangFrontend
        clangRewriteFrontend
        clangSerialization
      )
    elseif(${pkg} STREQUAL fmt)
      CPMAddPackage("gh:fmtlib/fmt#12.1.0")
      target_link_libraries("${target}" PRIVATE fmt::fmt)
    endif()
  endforeach()
endfunction()
target_compile_features(common-config INTERFACE cxx_std_23)
if(${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} STREQUAL GNU)
  target_compile_options(common-config INTERFACE
    -Wall
    -Werror
    $<$<COMPILE_LANGUAGE:CXX>:
      -fno-rtti
      -fno-exceptions
    >
  )
  set(asan "-fsanitize=address")
elseif(${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} STREQUAL MSVC)
  target_compile_options(common-config INTERFACE
    /W4
    /WX
    /utf-8
    $<$<COMPILE_LANGUAGE:CXX>:
      /GR-
      /EHsc
    >
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

  # Native MSVC
  if(MSVC AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_link_options($<$<CONFIG:Debug>:/INCREMENTAL:NO>)
  else()
    add_link_options($<$<CONFIG:Debug>:${asan}>)
  endif()
endif()
set(ubsan "-fsanitize=undefined")
set(CMAKE_REQUIRED_LINK_OPTIONS ${ubsan})
check_cxx_compiler_flag(${ubsan} hasUbsan)
if(hasUbsan)
  add_compile_options($<$<CONFIG:Debug>:${ubsan}>)
  add_link_options($<$<CONFIG:Debug>:${ubsan}>)
endif()

# Still needed bc of dynamic linking to UCRT and libSystem
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Since Clang was built with these settings, we must match them even in Debug mode
add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_subdirectory(src/importizer)
add_subdirectory(src/utils)
option(BUILD_TESTING "Setup and run tests" ON)
if(BUILD_TESTING)
  add_subdirectory(src/run-test)
  add_subdirectory(tests)
endif()
