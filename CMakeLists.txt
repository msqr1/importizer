cmake_minimum_required(VERSION 4.0.0)
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

set(root "${CMAKE_SOURCE_DIR}")
set(3rdPartyDir "${root}/3rd-party")
set(cmakeDir "${root}/cmake")
set(scriptsDir "${root}/scripts")
set(arch $ENV{IMPORTIZER_ARCH})

# Project
option(BUILD_TESTING "Build test driver and enable testing" FALSE)
option(TRACE_BUILD "Enable build time tracing" FALSE)
option(USE_PCH "Use precompiled headers to speed up repeated builds" FALSE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_SCAN_FOR_MODULES FALSE)
set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_library(common-config INTERFACE)
target_compile_features(common-config INTERFACE cxx_std_23)
target_include_directories(common-config INTERFACE "${root}/src")
target_compile_options(common-config INTERFACE
  -Wall
  -Wextra
  -Wpedantic
  -Wconversion
  -Wimplicit-fallthrough
  -Wshadow
  -Wold-style-cast
  -Wsuggest-override
  -Wnon-virtual-dtor
  -Wundef
  -Wextra-semi-stmt
  -Wglobal-constructors
  -Werror
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-unsafe-buffer-usage
  -Wno-switch-enum
  -Wno-switch-default
  -Wno-padded
)

if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  target_compile_options(common-config INTERFACE
    -ffunction-sections
    -fdata-sections
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti;-fno-exceptions;-fno-c++-static-destructors>"
    "-ffile-prefix-map=${root}/src/="
  )
  if(TRACE_BUILD)
    target_compile_options(common-config INTERFACE
      -ftime-trace
      -ftime-trace-granularity=1
    )
  endif()
elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  target_compile_options(common-config INTERFACE
    /Gy
    "$<$<COMPILE_LANGUAGE:CXX>:/GR-;/EHs-c-;/clang:-fno-c++-static-destructors>"
    "/clang:-ffile-prefix-map=${root}/src/="
  )
  if(TRACE_BUILD)
    target_compile_options(common-config INTERFACE
      /clang:-ftime-trace
      /clang:-ftime-trace-granularity=1
    )
  endif()
  target_link_options(common-config INTERFACE /OPT:REF)
endif()

if(LINUX)
  target_compile_options(common-config INTERFACE "$<$<CONFIG:Debug>:-fsanitize=address,undefined>")
  target_link_options(common-config INTERFACE
    "LINKER:--gc-sections,--as-needed,--no-copy-dt-needed-entries"

    # In Debug mode, we want to still link in whatever dynamic library we need.
    "$<$<CONFIG:Debug>:-static-libgcc;-static-libstdc++>"

    # In Release mode, we want a fully static. But instead of -static we use -static-pie for more
    # security. On Windows and MacOS things are already secure (PIE always on).
    "$<$<CONFIG:Release>:-static-pie;LINKER:-s>"
  )
elseif(APPLE)
  target_compile_options(common-config INTERFACE "$<$<CONFIG:Debug>:-fsanitize=address,undefined>")
  target_link_options(common-config INTERFACE
    "$<$<CONFIG:Release>:LINKER:-S,-x,-dead_strip>"
  )
elseif(WIN32 AND arch STREQUAL "amd64")
  set(clangArch x86_64)
  execute_process(
    COMMAND "${CMAKE_C_COMPILER}" --print-resource-dir
    OUTPUT_VARIABLE resourceDir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  target_compile_options(common-config INTERFACE "$<$<CONFIG:Debug>:-fsanitize=address,undefined>")
  # Windows linkers doesn't know where clang resource directory is, but we need that for
  # sanitizers
  set(clangLibPath "${resourceDir}/lib/windows")

  # Secret, undocumented define to make non-MSVC ASAN work.
  target_compile_definitions(common-config INTERFACE "$<$<CONFIG:Debug>:_DISABLE_STL_ANNOTATION>")

  target_link_options(common-config INTERFACE
    "$<$<CONFIG:Debug>:-libpath:${clangLibPath};clang_rt.asan_dynamic-${clangArch}.lib>"
    "$<$<CONFIG:Debug>:-wholearchive:clang_rt.asan_static_runtime_thunk-${clangArch}.lib>"
  )
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# 3rd-party and its utils (this is long so it's in another file)
include("${cmakeDir}/3rdParty.cmake")

# Precompiled headers and its utils
if(USE_PCH)
  add_library(PCHLib OBJECT "${root}/src/Empty.cc")
  target_link_libraries(PCHLib PRIVATE common-config)
  target_precompile_headers(PCHLib PRIVATE
    <vector>
    <chrono>
  )
  target_compile_definitions(PCHLib PRIVATE
    CLANG_BUILD_STATIC=1
  )
endif()
function(requirePCH target)
  if(USE_PCH)
    target_precompile_headers(${target} REUSE_FROM PCHLib)
  endif()
endfunction()

if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
add_subdirectory(src/importizer)
add_subdirectory(src/utils)

if(BUILD_TESTING)
  add_subdirectory(src/testing)
  add_subdirectory(tests)
endif()
