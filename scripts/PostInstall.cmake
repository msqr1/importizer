# Post-installation steps
# For use inclusion in CMake install rules

if(LINUX)
  file(RENAME "${exe}" "${CMAKE_INSTALL_PREFIX}/importizer.bin")
  file(
    GLOB loaders
    LIST_DIRECTORIES false
    "${CMAKE_INSTALL_PREFIX}/ld-*.so*"
  )
  list(GET loaders 0 loader)
  cmake_path(GET loader FILENAME loaderName)

  # Trampoline to load the exe with the included loader
  # We use string(CONCAT ) to be able to use raw strings
  string(CONCAT trampoline
[[#!/usr/bin/env sh
dir="$(cd "$(dirname "$0")" && pwd)"
exec "$dir/]]
  ${loaderName}
[[" --library-path "$dir" "$dir/importizer.bin" "$@"]]
  )
  file(WRITE "${exe}" "${trampoline}")
endif()

file(CHMOD_RECURSE "${CMAKE_INSTALL_PREFIX}" FILE_PERMISSIONS
  OWNER_READ
  OWNER_WRITE
  OWNER_EXECUTE
  GROUP_READ
  GROUP_EXECUTE
  WORLD_READ
  WORLD_WRITE
  WORLD_EXECUTE
)
