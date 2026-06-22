# Post-installation steps
# For use in CMake install rules, do not run alone

if(LINUX)
  file(RENAME "${exe}" "${CMAKE_INSTALL_PREFIX}/importizer.bin")
  file(
    GLOB loaders
    LIST_DIRECTORIES false
    "${CMAKE_INSTALL_PREFIX}/ld-*.so*"
  )
  list(GET loaders 0 loader)
  get_filename_component(loaderName "${loader}" NAME)

  # Trampoline to load the exe with the included loader
  # We use string(CONCAT ) to be able to use raw strings
  string(CONCAT trampoline
[[#!/usr/bin/env sh
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/]]
  ${loaderName}
[[" --library-path "$DIR" "$DIR/importizer.bin" "$@"]]
  )
  file(WRITE "${exe}" "${trampoline}")
endif()

# Make sure the executable lives up to its kind
file(CHMOD "${exe}" PERMISSIONS
  OWNER_READ
  OWNER_WRITE
  OWNER_EXECUTE
  GROUP_READ
  GROUP_EXECUTE
  WORLD_READ
  WORLD_WRITE
  WORLD_EXECUTE
)
