# Utility to speed up repeated builds: precompiled headers and ThinLTO cache

get_property(runMode GLOBAL PROPERTY CMAKE_ROLE)
if(NOT runMode STREQUAL "PROJECT")
  message(FATAL_ERROR "File must run in project mode.")
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
    if(LINUX)
      target_link_options(common-config INTERFACE
        "LINKER:--thinlto-cache-policy=${LTO_CACHE_POLICY}"
      )
    # MacOS's ld-prime doesn't allow specifying cache policy
    elseif(WIN32)
      target_link_options(common-config INTERFACE
        "LINKER:/lldltocachepolicy:${LTO_CACHE_POLICY}"
      )
    endif()

  endif()
endif()
