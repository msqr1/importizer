# Library for run-test
add_library(importizer-lib
  Opts.cc
  Toml.cc
  Main.cc
)
target_link_libraries(importizer-lib PRIVATE
  common-config
  utils-lib
)
require(importizer-lib PRIVATE LLVM Clang tomlc17)
# TODO: Use PCH here as well

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
target_precompile_headers(importizer REUSE_FROM pch-LLVM)

if(LINUX)
  add_custom_command(
    TARGET importizer
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -P "${scriptsDir}/PostBuild.cmake"
      "$<TARGET_FILE:importizer>"
      "${CMAKE_BINARY_DIR}/importizer"
    VERBATIM
  )

  # For Linux, we want to actually install system libraries if needed
  install(
    TARGETS importizer
    DESTINATION .
    RUNTIME_DEPENDENCIES
    DIRECTORIES /lib # TODO: Avoid hardcoding?
  )
elseif(WIN32)
  add_custom_command(
    TARGET importizer
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -P "${scriptsDir}/PostBuild.cmake"
      "$<TARGET_FILE:importizer>"
      "${CMAKE_BINARY_DIR}/importizer"
      "${resourceDir}\\lib\\windows"
    VERBATIM
  )

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
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  add_custom_command(
    TARGET importizer
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -P "${scriptsDir}/PostBuild.cmake"
      "$<TARGET_FILE:importizer>"
      "${CMAKE_BINARY_DIR}/importizer"
    VERBATIM
  )

  # Exclude MacOS CStdlib & CxxStdlib
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
include(\"${scriptsDir}/PostInstall.cmake\")
")
