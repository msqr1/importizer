# Post-build script
# For use as script in CMake post-build

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
  message(FATAL_ERROR "File must be run as CMake script.")
endif()

cmake_path(GET BIN_PATH PARENT_PATH binDir)

if(POLICY CMP0205)
  cmake_policy(SET CMP0205 NEW)
endif()
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

# Windows has no rpath so we symlink DLL's into the binary's directory post-build.
# For post-install, CMake install(RUNTIME_DEPENDENCIES) does the same thing.
if(WIN32)
  # Exclude Windows UCRT
  file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR deps
    EXECUTABLES "${BIN_PATH}"
    PRE_EXCLUDE_REGEXES
      [[^api-ms-.*]]
      [[^ext-ms-.*]]
    POST_EXCLUDE_REGEXES
      [[^.*[\\/]system32[\\/].*]]
    DIRECTORIES "${SEARCH_DIRS}"
  )
  foreach(dep IN LISTS deps)
    cmake_path(GET dep FILENAME depName)
    if(IS_READABLE "${binDir}/${depName}")
      continue()
    endif()
    file(CREATE_LINK "${dep}" "${binDir}/${depName}" SYMBOLIC)
  endforeach()
endif()

file(CREATE_LINK "${binDir}" "${PREDICTABLE_DIR}" SYMBOLIC)
