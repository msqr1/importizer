# Library for testing
add_library(importizer-lib
  Config.cc
  Main.cc
  PP.cc
  AST.cc
  Frontend.cc
  ClangDiag.cc
)
target_link_libraries(importizer-lib PRIVATE
  common-config
  utils-lib
)
require(importizer-lib PRIVATE LLVM Clang)
usePCH(importizer-lib)

# Executable for users
add_executable(importizer
  Driver.cc
)
target_link_libraries(importizer PRIVATE
  common-config
  utils-lib
  importizer-lib
)
require(importizer PRIVATE LLVM)
usePCH(importizer)

add_custom_target(importizer-post-build ALL
  COMMAND "${CMAKE_COMMAND}"
    "-DSEARCH_DIRS=$<$<CONFIG:Debug>:${clangLibPath}>"
    "-DBIN_PATH=$<TARGET_FILE:importizer>"
    "-DPREDICTABLE_DIR=${CMAKE_BINARY_DIR}/importizer"
    -P "${scriptsDir}/PostBuild.cmake"
  COMMENT ""
  VERBATIM
)
add_dependencies(importizer-post-build importizer)

if(LINUX)
  # Install CStdlib because already having musl is rare (added /lib).
  install(
    TARGETS importizer
    DESTINATION .
    RUNTIME_DEPENDENCIES
    DIRECTORIES /lib
  )
elseif(WIN32)
  # Exclude Windows UCRT, every Windows machine have it.
  install(
    TARGETS importizer
    DESTINATION .
    RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES
      [[^api-ms-.*]]
      [[^ext-ms-.*]]
    POST_EXCLUDE_REGEXES
      [[^.*[\\/]system32[\\/].*]]
  )
elseif(APPLE)
  # Exclude MacOS CStdlib and CxxStdlib, every MacOS machine have them.
  install(
    TARGETS importizer
    DESTINATION .
    RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES
      [[^libSystem\.B\.dylib]]
      [[^libc\+\+\.1\.dylib]]
  )
endif()
install(CODE "
set(binPath \"\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:importizer>\")
include(\"${scriptsDir}/PostInstall.cmake\")
")
