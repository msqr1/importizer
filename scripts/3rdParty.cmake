# 3rd-party library utilities (downloading, linking, precompiled headers, etc.)
# For use in CMakeLists.txt, do not run alone

# 3rd-party libs
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

set(CPM_SOURCE_CACHE "${3rdPartyDir}")
if(NOT IS_READABLE "${scriptsDir}/Cpm.cmake")
  file(DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.42.3/CPM.cmake
    "${scriptsDir}/Cpm.cmake"
    EXPECTED_HASH SHA256=a609e875fd532b067174250f6abbc3dac22fe2d64869783fb1e80bda1625c844
  )
endif()
include("${scriptsDir}/Cpm.cmake")

# Setup a 3rd-party library for a target
function(require target visibility)
  foreach(pkg IN LISTS ARGN)
    if(pkg STREQUAL tomlc17)
      # R260517
      CPMAddPackage("gh:cktan/tomlc17#cb9bba39f2e63a9e67fa61d6c7521a184eb5fc38")
      target_sources(${target} ${visibility} "${tomlc17_SOURCE_DIR}/src/tomlc17.c")

      # SYSTEM to supress warnings
      target_include_directories(${target} SYSTEM ${visibility} "${tomlc17_SOURCE_DIR}/src")

      if(WIN32)
        set_source_files_properties("${tomlc17_SOURCE_DIR}/src/tomlc17.c"
          PROPERTIES COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS
        )
      endif()
    elseif(pkg STREQUAL LLVM)
      set(LLVM_DIR "${3rdPartyDir}/llvm/lib/cmake/llvm")
      find_package(LLVM REQUIRED)
      target_include_directories(${target} SYSTEM ${visibility} "${LLVM_INCLUDE_DIRS}")
      target_link_libraries(${target} ${visibility}
        LLVMSupport
      )
    elseif(pkg STREQUAL Clang)
      set(Clang_DIR "${3rdPartyDir}/clang/lib/cmake/clang")
      find_package(Clang REQUIRED)
      target_include_directories(${target} SYSTEM ${visibility} "${CLANG_INCLUDE_DIRS}")
      target_link_libraries(${target} ${visibility}
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

# Add a precompiled header set to a target

file(TOUCH "${CMAKE_BINARY_DIR}/Empty.c")
add_library(pch-LLVM STATIC "${CMAKE_BINARY_DIR}/Empty.c")
if(pchSet STREQUAL LLVM)
  set_target_properties(pch-LLVM PROPERTIES PRECOMPILE_HEADERS
    <llvm/ADT/StringRef.h>
    <llvm/Support/FileSystem.h>
    <llvm/Support/VirtualFileSystem.h>
  )
endif()

set(allPchSets "LLVM;")

function(requirePch target pchSet)
  if(NOT pchSet IN_LIST allPchSets)
    message(FATAL_ERROR "Unknown precompiled header set '${pchSet}'")
  endif()
  target_precompile_headers(${target} REUSE_FROM pch-${pchSet})
endfunction()
