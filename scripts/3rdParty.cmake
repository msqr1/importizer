# 3rd-party library utilities (downloading, linking, precompiled headers, etc.)
# For inclusion in CMakeLists.txt

# 3rd-party libs
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

include("${scriptsDir}/EnsureLibtooling.cmake")
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

# Setup a 3rd-party library for a target
function(require target visibility)
  foreach(pkg IN LISTS ARGN)
    if(pkg STREQUAL "LLVM")
      target_include_directories(${target} SYSTEM ${visibility} "${LLVM_INCLUDE_DIRS}")
      target_link_libraries(${target} ${visibility}
        LLVMSupport
      )
    elseif(pkg STREQUAL "Clang")
      target_include_directories(${target} SYSTEM ${visibility}
        "${LLVM_INCLUDE_DIRS}"
        "${CLANG_INCLUDE_DIRS}"
      )
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

add_library(pch-LLVM OBJECT "${CMAKE_SOURCE_DIR}/src/Empty.cc")
require(pch-LLVM PRIVATE LLVM)
target_link_libraries(pch-LLVM PRIVATE common-config)
target_precompile_headers(pch-LLVM PRIVATE
  <vector>
  <chrono>
)
