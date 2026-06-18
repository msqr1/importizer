# Make a distribution
## Uses Github CLI, needs GH_TOKEN env var

set(mode ${CMAKE_ARGV3})
set(knownModes "Debug;Release")
if(NOT mode IN_LIST knownModes)
  message(FATAL_ERROR "Unknown mode '${mode}', only Debug/Release is allowed")
endif()
set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptsDir)
file(REAL_PATH "${scriptsDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(buildDir "${root}/build")

include("${scriptsDir}/Exec.cmake")

exec(${CMAKE_COMMAND} -B "${buildDir}" -DCMAKE_BUILD_TYPE=${mode} --preset importizer-${os})
exec(${CMAKE_COMMAND} --build "${buildDir}" --config ${mode})

exec(${CMAKE_CTEST_COMMAND}
  --test-dir "${buildDir}/tests"
  -C ${mode}
  --verbose
  --progress
  --schedule-random
  -j ${procCnt}
)

if(${mode} STREQUAL Debug)
  # Format in debug mode: tzst(installation)
  exec(${CMAKE_COMMAND}
    --install "${buildDir}"
    --config ${mode}
    --prefix "${root}/importizer"
    -j ${procCnt}
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

if(NOT DEFINED ENV{CI})
  cmake_language(EXIT 0)
endif()

if(${mode} STREQUAL Debug)
  set(repo "msqr1/importizer")

  # Upload
  exec(gh release upload continuous "${root}/${os}-${arch}.tzst" --clobber -R ${repo})

  # Update Continous tag to current commit
  exec(git tag -f continuous)
  exec(git remote set-url --push origin https://msqr1:$ENV{GH_TOKEN}@github.com/${repo}.git)
  execUnsafe(git push -f origin continuous)
endif()

# CI release mode upload handled by actions/upload-artifact
