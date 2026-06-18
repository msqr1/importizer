# Make a distribution
## Uses Github CLI, needs GH_TOKEN env var

set(mode ${CMAKE_ARGV3})
set(knownModes "Debug;Release")
if(NOT mode IN_LIST knownModes)
  message(FATAL_ERROR "Unknown mode '${mode}', only Debug/Release is allowed")
endif()
set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptDir)
file(REAL_PATH "${scriptDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(buildDir "${root}/build")

execute_process(COMMAND ${CMAKE_COMMAND}
  -B "${buildDir}"
  -DCMAKE_BUILD_TYPE=${mode}
  --preset importizer-${os}
  COMMAND_ERROR_IS_FATAL ANY
)
execute_process(COMMAND ${CMAKE_COMMAND}
  --build "${buildDir}"
  --config ${mode}
  COMMAND_ERROR_IS_FATAL ANY
)

execute_process(COMMAND ${CMAKE_CTEST_COMMAND}
  --test-dir "${buildDir}/tests"
  -C ${mode}
  --verbose
  --progress
  --schedule-random
  -j ${procCnt}
  COMMAND_ERROR_IS_FATAL ANY
)

if(${mode} STREQUAL Debug)
  # Format in debug mode: tzst(bin + deps)
  execute_process(COMMAND ${CMAKE_COMMAND}
    --install "${buildDir}"
    --config ${mode}
    --prefix "${root}/importizer"
    -j ${procCnt}
    COMMAND_ERROR_IS_FATAL ANY
  )
  file(ARCHIVE_CREATE
    OUTPUT "${root}/${os}-${arch}.tzst"
    PATHS "${root}/importizer"
    COMPRESSION Zstd
    COMPRESSION_LEVEL 19
  )
elseif(${mode} STREQUAL Release)
  # Format in release mode: zst(bin)
  set(bin "importizer")
  if(WIN32)
    set(bin "${bin}.exe")
  endif()
  file(ARCHIVE_CREATE
    OUTPUT "${root}/${os}-${arch}.zst"
    FORMAT raw
    PATHS "${root}/build/${bin}"
    COMPRESSION Zstd
    COMPRESSION_LEVEL 19
  )
endif()

if(DEFINED ENV{CI} AND ${mode} STREQUAL Debug)
  set(repo "msqr1/importizer")

  # Upload & move tag
  execute_process(COMMAND gh release upload continuous "${root}/${os}-${arch}.tzst"
    --clobber
    -R ${repo}
    COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND git tag -f continuous COMMAND_ERROR_IS_FATAL ANY)

  # Authenticate & push
  execute_process(COMMAND git remote set-url
    --push
    origin
    https://msqr1:$ENV{GH_TOKEN}@github.com/${repo}.git
    COMMAND_ERROR_IS_FATAL ANY
  )
  execute_process(COMMAND git push -f origin continuous COMMAND_ERROR_IS_FATAL ANY)
endif()
