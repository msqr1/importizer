add_executable(importizer
  Driver.cc
  Opts.cc
  Toml.cc
)
require(importizer fmt Clang tomlc17)
if(WIN32)
  set(CPACK_GENERATOR "ZIP")
elseif(LINUX)
  # Static except for glibc, LLVM and its deps
  target_link_options(importizer PRIVATE
    -static-libgcc
    -static-libstdc++ # This can makes libc++ static, too.
  )
  set(CPACK_GENERATOR "TGZ")

  # Exclude Linux system libraries
  install(
    RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      "libm\\.so"
      "libc\\.so"
      "libdl\\.so"
      "libpthread\\.so"
      "librt\\.so"
      "libresolv\\.so"
      "libutil\\.so"
      "ld-linux.*\\.so"
      "libgcc_s\\.so"
      "libstdc\\+\\+\\.so"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # MacOS linking is handled by CMAKE_OSX_DEPLOYMENT_TARGET
  set(CPACK_GENERATOR "TGZ")

  # Exclude MacOS system libraries
  install(
    RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      "libSystem\\.B\\.dylib"
      "libc\\+\\+\\.1\\.dylib"
      "libobjc\\.dylib"
      "/usr/lib/swift"
    )
endif()
target_link_libraries(importizer PRIVATE
  common-config
  utils
)
install(
  TARGETS importizer
  DESTINATION .
  RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
)
if(NOT CPACK_PACKAGE_FILE_NAME)
  set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
endif()
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
include(CPack)

# For the developer to be able to keep staying in the build directory
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_FILE:importizer>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")
