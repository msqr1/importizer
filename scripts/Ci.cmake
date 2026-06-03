set(os ${CMAKE_ARGV0})
set(arch ${CMAKE_ARGV1})
set(mode ${CMAKE_ARGV2})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptDir)
file(REAL_PATH "${scriptDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(buildDir "${root}/build")

execute_process(COMMAND ${CMAKE_COMMAND} -B "${buildDir}" -DCMAKE_BUILD_TYPE=${mode}
  --preset importizer-${os} COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND ${CMAKE_COMMAND} --build "${buildDir}" --config ${mode}
  COMMAND_ERROR_IS_FATAL ANY)

execute_process(COMMAND ${CMAKE_CTEST_COMMAND} --test-dir "${buildDir}/tests" -C ${mode}
  --verbose --progress --schedule-random -j ${procCnt} COMMAND_ERROR_IS_FATAL ANY)

if(${mode} STREQUAL Debug)
  execute_process(COMMAND ${CMAKE_COMMAND} --install "${buildDir}" --config ${mode}
    --prefix "${root}/importizer" -j ${procCnt} COMMAND_ERROR_IS_FATAL ANY)
  file(ARCHIVE_CREATE OUTPUT "${root}/${os}-${arch}.tzst" PATHS "${root}/importizer"
    COMPRESSION Zstd COMPRESSION_LEVEL 19)

  # Upload & move tag
  execute_process(COMMAND gh release upload continuous "${root}/${os}-${arch}.tzst"
    --clobber -R "msqr1/importizer" COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND git tag -f continuous COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND git push -f origin continuous COMMAND_ERROR_IS_FATAL ANY)

  cmake_language(EXIT 0)
endif()

set(bin "importizer")
if(WIN32)
  set(bin "${bin}.exe")
endif()

# Leave to actions/upload-artifact
file(ARCHIVE_CREATE OUTPUT "${root}/${os}-${arch}.zst" FORMAT raw PATHS
  "${root}/build/${bin}" COMPRESSION Zstd COMPRESSION_LEVEL 19)
