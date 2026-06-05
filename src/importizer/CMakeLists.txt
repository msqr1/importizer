add_executable(importizer
  Driver.cc
  Opts.cc
  Toml.cc
)
require(importizer fmt Clang tomlc17)
if(WIN32)
  install(
    RUNTIME_DEPENDENCY_SET deps
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      [[^api-ms-.*]]
      [[^ext-ms-.*]]
    POST_EXCLUDE_REGEXES
      [[^.*[\\/]system32[\\/].*]]
  )
elseif(LINUX)
  # Static except for glibc, LLVM and its deps
  target_link_options(importizer PRIVATE
    -static-libgcc
    -static-libstdc++ # This can makes libc++ static, too.
  )

  # Exclude Linux system libraries
  install(
    RUNTIME_DEPENDENCY_SET deps
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      [[libm\.so]]
      [[libc\.so]]
      [[libdl\.so]]
      [[libpthread\.so]]
      [[librt\.so]]
      [[libresolv\.so]]
      [[libutil\.so]]
      [[ld-linux.*\.so]]
      [[libgcc_s\.so]]
      [[libstdc\+\+\.so]]
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # Exclude MacOS system libraries
  install(
    RUNTIME_DEPENDENCY_SET deps
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      [[^libSystem\.B\.dylib]]
      [[^libc\+\+\.1\.dylib]]
      [[^libobjc\.dylib]]
      [[^/usr/lib/swift]]
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
