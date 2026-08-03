# Post-build script
# For use as script in CMake post-build

cmake_path(GET exe PARENT_PATH exeDir)

if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()
if(WIN32)
  # Windows has no Rpath so we symlink DLL's into the executable's directory

  # Exclude Windows UCRT
  file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR deps
    EXECUTABLES "${exe}"
    PRE_EXCLUDE_REGEXES
      [[^api-ms-.*]]
      [[^ext-ms-.*]]
    POST_EXCLUDE_REGEXES
      [[^.*[\\/]system32[\\/].*]]
    DIRECTORIES "${searchDirs}"
  )
  foreach(dep IN LISTS deps)
    cmake_path(GET dep FILENAME depName)
    if(IS_READABLE "${exeDir}/${depName}")
      continue()
    endif()
    file(CREATE_LINK "${dep}" "${exeDir}/${depName}" COPY_ON_ERROR SYMBOLIC)
  endforeach()
endif()

file(CREATE_LINK "${exeDir}" "${predictableDir}" COPY_ON_ERROR SYMBOLIC)
