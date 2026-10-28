# Post-installation steps
# For use inside CMake installation script (cmake_install.cmake)

get_property(runMode GLOBAL PROPERTY CMAKE_ROLE)
if(NOT runMode STREQUAL "SCRIPT")
  message(FATAL_ERROR "File must run in cmake -P script mode.")
endif()

if(NOT DEFINED binPath)
  message(FATAL_ERROR "Missing binary's path.")
endif()

if(LINUX AND CMAKE_INSTALL_CONFIG_NAME STREQUAL "Debug")
  file(RENAME "${binPath}" "${CMAKE_INSTALL_PREFIX}/importizer.bin")
  file(
    GLOB loaders
    LIST_DIRECTORIES false
    "${CMAKE_INSTALL_PREFIX}/ld-*.so*"
  )
  list(GET loaders 0 loader)
  cmake_path(GET loader FILENAME loaderName)

  # Trampoline to load the binary with the included loader
  # We use string(CONCAT ) to be able to use raw strings
  string(CONCAT trampoline
[[#!/usr/bin/env sh
dir="$(cd "$(dirname "$0")" && pwd)"
exec "$dir/]]
  ${loaderName}
[[" --library-path "$dir" "$dir/importizer.bin" "$@"]]
  )
  file(WRITE "${binPath}" "${trampoline}")
endif()
