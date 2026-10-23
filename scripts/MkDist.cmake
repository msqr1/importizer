# Make a distribution from a build directory, must be already built and tested!
## Uses Github CLI, needs GH_TOKEN env var

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
  message(FATAL_ERROR "File must be run as CMake script.")
endif()

set(knownTypes "Debug;Release")
if(NOT type IN_LIST knownTypes)
  message(FATAL_ERROR "Unknown distribution type '${type}', only Debug/Release is allowed")
endif()
set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptsDir)
file(REAL_PATH "${scriptsDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)

set(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL TRUE)

if(type STREQUAL "Debug")
  # Debug distribution format: tzst(installation)
  execute_process(COMMAND "${CMAKE_COMMAND}"
    --install "${buildDir}"
    --config ${type}
    --prefix "${root}/importizer"
    -j ${procCnt}
  )
  file(ARCHIVE_CREATE
    OUTPUT "${root}/importizer-${os}-${arch}.tzst"
    PATHS "${root}/importizer"
    COMPRESSION Zstd
    COMPRESSION_LEVEL 19
  )
elseif(type STREQUAL "Release")
  # Release distribution format: zst(bin)
  set(bin "importizer")
  if(WIN32)
    set(bin "${bin}.exe")
  endif()
  file(ARCHIVE_CREATE
    OUTPUT "${root}/importizer-${os}-${arch}.zst"
    FORMAT raw
    PATHS "${root}/build/importizer/${bin}"
    COMPRESSION Zstd
    COMPRESSION_LEVEL 19
  )
endif()

if(NOT DEFINED ENV{CI})
  return()
endif()

if(type STREQUAL "Debug")
  set(repo "msqr1/importizer")

  # Upload
  execute_process(COMMAND
    gh release upload continuous "${root}/${os}-${arch}.tzst"
    --clobber
    -R ${repo}
  )

  # Update continous tag to current commit
  execute_process(COMMAND git tag -f continuous)
  execute_process(COMMAND
    git remote set-url --push
    origin https://msqr1:$ENV{GH_TOKEN}@github.com/${repo}.git\
  )
  execute_process(COMMAND
    git push -f origin continuous
    COMMAND_ERROR_IS_FATAL NONE
  )
endif()

# CI release mode upload handled by actions/upload-artifact
