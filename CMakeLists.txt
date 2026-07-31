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

set(root "${CMAKE_SOURCE_DIR}")
set(3rdPartyDir "${root}/3rd-party")
set(scriptsDir "${root}/scripts")
set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})

# Project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_library(common-config INTERFACE)
target_compile_features(common-config INTERFACE cxx_std_23)
target_include_directories(common-config INTERFACE "${CMAKE_SOURCE_DIR}/src")
set(compileFlags
  -Wall
  -Wextra
  -Wconversion
  -Wsign-conversion
  -Wimplicit-fallthrough
  -Werror
  -Wno-c++98-compat
  -Wno-unsafe-buffer-usage
  -Wno-switch-default
  -Wno-padded
  -fno-delete-null-pointer-checks
  -fno-strict-aliasing
  "$<$<CONFIG:Debug>:-fsanitize=address,undefined>"
)
set(linkFlags)

if(arch STREQUAL "amd64")
  set(clangArch x86_64)
  list(APPEND compileFlags -fcf-protection=full)
elseif(arch STREQUAL "arm64")
  set(clangArch aarch64)
  list(APPEND compileFlags -mbranch-protection=standard)
endif()

if(LINUX)
  list(APPEND linkFlags
    # In Debug mode, we want to still link in whatever dynamic library we need.
    "$<$<CONFIG:Debug>:-static-libgcc;-static-libstdc++>"

    # In Release mode, we want a fully static. But instead of -static we use -static-pie for more
    # security. On Windows & MacOS things are already secure (PIE always on).
    "$<$<CONFIG:Release>:-static-pie>"
  )
elseif(WIN32)
  # Windows linkers doesn't know where clang resource directory is, but we need that for sanitizers
  execute_process(
    COMMAND "${CMAKE_C_COMPILER}" --print-resource-dir
    OUTPUT_VARIABLE clangLibPath
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  set(clangLibPath "${clangLibPath}/lib/windows") # Empty if not on Windows

  # Secret, undocumented define to make non-MSVC ASAN work.
  list(APPEND compileFlags "$<$<CONFIG:Debug>:-D_DISABLE_STL_ANNOTATION>")

  list(APPEND linkFlags
    "$<$<CONFIG:Debug>:-libpath:${clangLibPath};clang_rt.asan_dynamic-${clangArch}.lib;-wholearchive:clang_rt.asan_static_runtime_thunk-${clangArch}.lib>"
  )
endif()

if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  list(APPEND compileFlags
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti;-fno-exceptions>"
    -fvisibility=hidden
    -funique-source-file-names
    -fstrict-flex-arrays=3
    -fstack-clash-protection
    -fstack-protector-all
  )
  list(APPEND linkFlags
    "-Wl,-z,nodlopen,-z,noexecstack,-z,relro,-z,now,--as-needed,--no-copy-dt-needed-entries"
    "$<$<CONFIG:Release>:-Wl,-s>"
  )
elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  list(APPEND compileFlags
    "$<$<COMPILE_LANGUAGE:CXX>:/GR-;/EHs-c->"
    /sdl
    /guard:cf
  )
  list(APPEND linkFlags
    /CETCOMPAT
    /GUARD:CF
  )

  # Since LibTooling was built the release runtime, we must match it even in Debug mode
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

option(TRACE_BUILD "Enable build tracing for use with something like ClangBuildAnalyzer" OFF)
if(TRACE_BUILD)
  list(APPEND compileFlags -ftime-trace -ftime-trace-granularity=1)
endif()

target_compile_options(common-config INTERFACE ${compileFlags})
target_link_options(common-config INTERFACE ${linkFlags})

# 3rd-party
include("${scriptsDir}/3rdParty.cmake")

if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
add_subdirectory(src/importizer)
add_subdirectory(src/utils)

option(BUILD_TESTING "Build test driver & enable tests" OFF)
if(BUILD_TESTING)
  add_subdirectory(src/run-test)
  add_subdirectory(tests)
endif()
