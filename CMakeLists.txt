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
set(arch $ENV{IMPORTIZER_ARCH})

# Project
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  add_compile_options(
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti;-fno-exceptions>"
  )
  add_link_options(
    # In Debug mode, we want to still link in whatever dynamic library we need.
    "$<$<CONFIG:Debug>:-static-libgcc;-static-libstdc++>"

    # In Release mode, we want a fully static. But instead of -static we use -static-pie for more
    # security. On Windows & MacOS things are already secure (PIE always on).
    "$<$<CONFIG:Release>:-static-pie>"
  )
elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  add_compile_options(
    "$<$<COMPILE_LANGUAGE:CXX>:/GR-;/EHs-c->"
  )

  # Since LibTooling was built the release runtime, we must match it even in Debug mode
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()
if(POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
endif()
include(CheckIPOSupported)
check_ipo_supported(RESULT res)
if(res)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

include(CheckCompilerFlag)
include(CMakePushCheckState)

set(asan -fsanitize=address)
if(WIN32)
  # Windows linkers doesn't know where clang resource directory is, but we need that for ASAN
  execute_process(
    COMMAND "${CMAKE_C_COMPILER}" --print-resource-dir
    OUTPUT_VARIABLE resourceDir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  if(arch STREQUAL amd64)
    set(clangArch x86_64)
  elseif(arch STREQUAL arm64)
    set(clangArch aarch64)
  endif()
  set(winAsanLdFlags
    "-libpath:${resourceDir}\\lib\\windows"
    clang_rt.asan_dynamic-${clangArch}.lib
    -wholearchive:clang_rt.asan_static_runtime_thunk-${clangArch}.lib
  )
  set(CMAKE_REQUIRED_LINK_OPTIONS ${winAsanLdFlags})
else()
  set(CMAKE_REQUIRED_LINK_OPTIONS ${asan})
endif()
check_compiler_flag(C ${asan} hasAsan)
cmake_reset_check_state()
if(hasAsan)
  add_compile_options("$<$<CONFIG:Debug>:${asan}>")
  if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    add_link_options("$<$<CONFIG:Debug>:${asan}>")
  elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # Secret, undocumented definition to make non-MSVC ASAN work.
    add_compile_definitions("$<$<CONFIG:Debug>:_DISABLE_STL_ANNOTATION>")
  endif()
  if(WIN32)
    add_link_options("$<$<CONFIG:Debug>:${winAsanLdFlags}>")
  endif()
endif()

set(ubsan -fsanitize=undefined)
check_compiler_flag(C ${ubsan} hasUbsan)
cmake_reset_check_state()
if(hasUbsan)
  add_compile_options("$<$<CONFIG:Debug>:${ubsan}>")
endif()

set(lsan -fsanitize=leak)
set(CMAKE_REQUIRED_LINK_OPTIONS ${lsan})
check_compiler_flag(C ${lsan} hasLsan)
cmake_reset_check_state()
if(hasLsan)
  add_compile_options("$<$<CONFIG:Debug>:${lsan}>")
  add_link_options("$<$<CONFIG:Debug>:${lsan}>")
endif()

option(TRACE_BUILD "Enable build tracing for use with something like ClangBuildAnalyzer" OFF)
if(TRACE_BUILD)
  add_compile_options(
    -ftime-trace
    -ftime-trace-granularity=1
  )
endif()

add_library(common-config INTERFACE)
target_compile_features(common-config INTERFACE cxx_std_23)
target_include_directories(common-config INTERFACE "${CMAKE_SOURCE_DIR}/src")
target_compile_options(common-config INTERFACE
  -Wall
  -Wextra
  -Wno-c++98-compat
  -Wno-unsafe-buffer-usage
  -Wno-switch-default
  -Werror
)

# 3rd-party
include("${scriptsDir}/3rdParty.cmake")

if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
add_subdirectory(src/importizer)
add_subdirectory(src/utils)

option(BUILD_TESTING "Build test driver & run tests" OFF)
if(BUILD_TESTING)
  add_subdirectory(src/run-test)
  add_subdirectory(tests)
endif()
