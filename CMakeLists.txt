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
set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})

# Build customization settings
option(BUILD_TESTING "Build test driver and enable testing." FALSE)
option(TRACE_BUILD "Enable build time tracing for something like ClangBuildAnalyzer" FALSE)
option(USE_PCH "Use precompiled headers to speed up repeated compilation." FALSE)
option(LTO_CACHE "Cache ThinLTO results to speed up repeated linking." FALSE)
set(LTO_CACHE_POLICY "cache_size_bytes=1g" CACHE STRING
  "Set the ThinLTO cache policy if LTO_CACHE is on, no effect on MacOS.")

# Project's common configuration
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
    "-ffile-prefix-map=${root}/src/="
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti;-fno-exceptions;-fno-c++-static-destructors>"
    "$<$<CONFIG:Debug>:-fsanitize=address,undefined;-fstandalone-debug>"
  )
  target_link_options(common-config INTERFACE
    "$<$<CONFIG:Debug>:-fsanitize=address,undefined>"
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
    "/clang:-ffile-prefix-map=${root}/src/="
    "$<$<COMPILE_LANGUAGE:CXX>:/GR-;/EHs-c-;/clang:-fno-c++-static-destructors>"
  )
  if(TRACE_BUILD)
    target_compile_options(common-config INTERFACE
      /clang:-ftime-trace
      /clang:-ftime-trace-granularity=1
    )
  endif()
endif()

if(LINUX)
  target_link_options(common-config INTERFACE
    "LINKER:--gc-sections,--as-needed,--no-copy-dt-needed-entries"

    # In Debug mode, we want to still link in whatever dynamic library we need.
    "$<$<CONFIG:Debug>:-static-libgcc;-static-libstdc++>"

    # In Release mode, we want a fully static. But instead of -static we use -static-pie for more
    # security. On Windows and MacOS things are already secure (PIE always on).
    "$<$<CONFIG:Release>:-static-pie;LINKER:-s>"
  )
elseif(APPLE)
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

# 3rd-party and its utils
include("${cmakeDir}/3rdParty.cmake")

# Utils to speed up builds
include("${cmakeDir}/BuildSpeedup.cmake")

if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

add_subdirectory(src/utils)
add_subdirectory(src/importizer)
if(BUILD_TESTING)
  add_subdirectory(src/testing)
  add_subdirectory(tests)
endif()
