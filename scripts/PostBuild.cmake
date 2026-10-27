# Post-build script

get_property(runMode GLOBAL PROPERTY CMAKE_ROLE)
if(NOT runMode STREQUAL "SCRIPT")
  message(FATAL_ERROR "File must run in cmake -P script mode.")
endif()

if(NOT DEFINED BIN_PATH)
  message(FATAL_ERROR "Missing binary's path, specify one with -DBIN_PATH=.")
endif()

if(NOT DEFINED PREDICTABLE_DIR)
  message(FATAL_ERROR "Missing predictable directory, specify one with -DPREDICTABLE_DIR=.")
endif()

cmake_path(GET BIN_PATH PARENT_PATH binDir)

if(POLICY CMP0205)
  cmake_policy(SET CMP0205 NEW)
endif()
if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

# Windows has no rpath so we symlink deps next to the binary post-build.
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

    # When there are deps next to the binary (possibly from previous runs of this script),
    # file(GET_RUNTIME_DEPENDENCIES) resolves to it instead so symlinking would be to the same file
    if(NOT dep STREQUAL "${binDir}/${depName}")
      file(CREATE_LINK "${dep}" "${binDir}/${depName}" SYMBOLIC)
    endif()
  endforeach()
endif()

file(CREATE_LINK "${binDir}" "${PREDICTABLE_DIR}" SYMBOLIC)
