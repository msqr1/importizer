# 3rd-party library utilities
# For inclusion in CMakeLists.txt

include_guard(GLOBAL)

# 3rd-party libs
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

# Setup a 3rd-party library for a target
include("${cmakeDir}/EnsureLibTooling.cmake")
include("${cmakeDir}/EnsureDoctest.cmake")

function(require target visibility)
  foreach(pkg IN LISTS ARGN)
    if(pkg STREQUAL "LLVM")
      find_package(LLVM REQUIRED)
      target_include_directories(${target} SYSTEM ${visibility} "${LLVM_INCLUDE_DIRS}")
      target_link_libraries(${target} ${visibility}
        LLVMSupport
      )
    elseif(pkg STREQUAL "Clang")
      find_package(LLVM REQUIRED)
      find_package(Clang REQUIRED)
      target_include_directories(${target} SYSTEM ${visibility}
        "${LLVM_INCLUDE_DIRS}"
        "${CLANG_INCLUDE_DIRS}"
      )
      target_link_libraries(${target} ${visibility}
        clangAST
        clangBasic
        clangTooling
        clangFrontend
      )
    elseif(pkg STREQUAL "doctest")
      find_package(doctest REQUIRED)
      target_link_libraries(${target} ${visibility} doctest::doctest)
    else()
      message(FATAL_ERROR "Unknown 3rd-party library '${pkg}'")
    endif()
  endforeach()
endfunction()
