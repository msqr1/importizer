# Library for testing
add_library(importizer-lib
  Opts.cc
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
requirePCH(importizer-lib)

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
requirePCH(importizer)

add_custom_command(
  TARGET importizer
  POST_BUILD
  COMMAND "${CMAKE_COMMAND}"
    "-DsearchDirs=$<$<CONFIG:Debug>:${clangLibPath}>"
    "-Dbin=$<TARGET_FILE:importizer>"
    "-DpredictableDir=${CMAKE_BINARY_DIR}/importizer"
    -P "${scriptsDir}/PostBuild.cmake"
  VERBATIM
)

if(LINUX)
  # For Linux, we want to actually install system libraries if needed
  install(
    TARGETS importizer
    DESTINATION .
    RUNTIME_DEPENDENCIES
    DIRECTORIES /lib # TODO: Avoid hardcoding?
  )
elseif(WIN32)
  # Exclude Windows UCRT
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
  # Exclude MacOS CStdlib and CxxStdlib
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
set(exe \"\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:importizer>\")
include(\"${cmakeDir}/PostInstall.cmake\")
")
