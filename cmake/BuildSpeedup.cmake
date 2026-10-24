# Utility to speed up repeated builds: precompiled headers and ThinLTO cache

if(DEFINED CMAKE_SCRIPT_MODE_FILE)
  message(FATAL_ERROR "File unsuitable to be run as CMake script.")
endif()
include_guard(GLOBAL)

if(USE_PCH)
  add_library(PCHLib OBJECT "${root}/src/Empty.cc")
  target_link_libraries(PCHLib PRIVATE common-config)
  require(PCHLib PRIVATE LLVM)

  # We can only put headers all target uses, which currently are from
  # CStdlib, CXXStdlib, LLVM, and src/utils/
  # To know what to add, use something like ClangBuildAnalyzer
  target_precompile_headers(PCHLib PRIVATE
    <vector>
    <chrono>
    <llvm/ADT/StringRef.h>
    <llvm/ADT/DenseMapInfo.h>
    [["utils/Diag.hh"]]
  )
  target_compile_definitions(PCHLib PRIVATE
    CLANG_BUILD_STATIC=1
  )
endif()

# Use PCH for a target
function(usePCH target)
  if(USE_PCH)
    target_precompile_headers(${target} REUSE_FROM PCHLib)
  endif()
endfunction()

if(LTO_CACHE)
  set(ltoCacheDir "${CMAKE_BINARY_DIR}/lto-cache")
  if(LINUX)
    target_link_options(common-config INTERFACE "LINKER:--thinlto-cache-dir=${ltoCacheDir}")
  elseif(APPLE)
    target_link_options(common-config INTERFACE "LINKER:-cache_path_lto,${ltoCacheDir}")
  elseif(WIN32)
    target_link_options(common-config INTERFACE "LINKER:/lldltocache:${ltoCacheDir}")
  endif()
  if(NOT LTO_CACHE_POLICY STREQUAL "")
    if(LINUX OR APPLE)
      target_link_options(common-config INTERFACE
        "LINKER:--thinlto-cache-policy=${LTO_CACHE_POLICY}"
      )
    elseif(WIN32)
      target_link_options(common-config INTERFACE
        "LINKER:/lldltocachepolicy:${LTO_CACHE_POLICY}"
      )
    endif()
  endif()
endif()
