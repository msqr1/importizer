# Library for run-test
add_library(importizer-lib
  Opts.cc
  Toml.cc
  Entry.cc
)
target_link_libraries(importizer-lib PRIVATE
  common-config
  utils
)
require(importizer-lib PRIVATE fmt Clang tomlc17)

# Executable for users
add_executable(importizer
  Driver.cc
)
target_link_libraries(importizer PRIVATE
  common-config
  utils
  importizer-lib
)
require(importizer PRIVATE fmt)
if(LINUX)
  target_link_options(importizer PRIVATE -static)
elseif(WIN32)
  # Exclude Windows UCRT
  install(
    RUNTIME_DEPENDENCY_SET deps
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      [[^api-ms-.*]]
      [[^ext-ms-.*]]
    POST_EXCLUDE_REGEXES
      [[^.*[\\/]system32[\\/].*]]
  )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Exclude MacOS CStdlib & CxxStdlib
  install(
    RUNTIME_DEPENDENCY_SET deps
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      [[^libSystem\.B\.dylib]]
      [[^libc\+\+\.1\.dylib]]
  )
endif()
install(
  TARGETS importizer
  DESTINATION .
  RUNTIME_DEPENDENCY_SET deps
)

# More predictable output location
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E create_hardlink "$<TARGET_FILE:importizer>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")
