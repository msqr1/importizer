add_executable(importizer
  Driver.cc
  Opts.cc
  Toml.cc
)
require(importizer fmt Clang tomlc17)
if(WIN32)

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
  set_target_properties(importizer PROPERTIES INSTALL_RPATH "@executable_path")

  # Exclude MacOS CStdlib & CxxStdlib
  install(
    RUNTIME_DEPENDENCY_SET deps
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      [[^libSystem\.B\.dylib]]
      [[^libc\+\+\.1\.dylib]]
  )
endif()
target_link_libraries(importizer PRIVATE
  common-config
  utils
)
install(
  TARGETS importizer
  DESTINATION .
  RUNTIME_DEPENDENCY_SET deps
)

# More predictable output location
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E create_hardlink "$<TARGET_FILE:importizer>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")
