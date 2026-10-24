# Post-installation steps

if(DEFINED CMAKE_SCRIPT_MODE_FILE)
  message(FATAL_ERROR "File unsuitable to be run as CMake script.")
endif()
include_guard(GLOBAL)

if(LINUX)
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

file(CHMOD_RECURSE "${CMAKE_INSTALL_PREFIX}" FILE_PERMISSIONS
  OWNER_READ
  OWNER_WRITE
  OWNER_EXECUTE
  GROUP_READ
  GROUP_WRITE
  GROUP_EXECUTE
  WORLD_READ
  WORLD_WRITE
  WORLD_EXECUTE
)
