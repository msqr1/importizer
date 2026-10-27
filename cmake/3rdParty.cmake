# 3rd-party library utilities

get_property(runMode GLOBAL PROPERTY CMAKE_ROLE)
if(NOT runMode STREQUAL "PROJECT")
  message(FATAL_ERROR "File must run in project mode.")
endif()
include_guard(GLOBAL)

include("${cmakeDir}/EnsureLibTooling.cmake")
include("${cmakeDir}/EnsureDoctest.cmake")

# Setup a 3rd-party library for a target
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
