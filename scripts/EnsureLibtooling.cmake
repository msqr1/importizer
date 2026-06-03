set(os ${CMAKE_ARGV0})
set(arch ${CMAKE_ARGV1})
set(mode ${CMAKE_ARGV2})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptDir)
file(REAL_PATH "${scriptDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(v 22.1.6)
set(3rdPartyDir "${root}/3rd-party")
set(libtoolingSrc "${scriptDir}/libtooling-src")
set(repo "msqr1/importizer")

function(getSrc)
  set(arFile "${scriptDir}/Src.tar.xz")

  file(DOWNLOAD
    https://github.com/llvm/llvm-project/releases/download/llvmorg-${v}/llvm-project-${v}.src.tar.xz
    "${arFile}"
    EXPECTED_HASH SHA256=6e0b376a1f6d9873e7dfb09ae6e04b9c7024400f01733fa4c29be69d5c138bc2
    SHOW_PROGRESS STATUS status
  )
  list(GET status 0 errCode)
  if(NOT errCode EQUAL 0)
    list(GET status 1 err)
    message(FATAL_ERROR "Download failed: ${err}")
  endif()

  file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${scriptDir}")
  file(RENAME "${scriptDir}/llvm-project-${v}.src" "${libtoolingSrc}")

  file(CREATE_LINK "${root}/CMakePresets.json"
    "${libtoolingSrc}/llvm/CMakeUserPresets.json" COPY_ON_ERROR)
  file(CREATE_LINK "${root}/CMakePresets.json"
    "${libtoolingSrc}/clang/CMakeUserPresets.json" COPY_ON_ERROR)

  file(REMOVE "${arFile}")
endfunction()

function(build)
  file(MAKE_DIRECTORY "${3rdPartyDir}")
  execute_process(COMMAND ${CMAKE_COMMAND} -S "${libtoolingSrc}/llvm"
    -B "${libtoolingSrc}/build" --preset libtooling-${os} --no-warn-unused-cli
    COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND ${CMAKE_COMMAND} --build "${libtoolingSrc}/build"
    COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND ${CMAKE_COMMAND} --install "${libtoolingSrc}/build" --prefix
    "${3rdPartyDir}/llvm" -j ${procCnt} COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND ${CMAKE_COMMAND} -S "${libtoolingSrc}/clang"
    -B "${libtoolingSrc}/build2" --preset libtooling-windows
    -DLLVM_DIR="${3rdPartyDir}/llvm/lib/cmake/llvm" --no-warn-unused-cli
    COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND ${CMAKE_COMMAND} --build "${libtoolingSrc}/build2"
    COMMAND_ERROR_IS_FATAL ANY)
  execute_process(COMMAND ${CMAKE_COMMAND} --install "${libtoolingSrc}/build2" --prefix
    "${3rdPartyDir}/clang" -j ${procCnt} COMMAND_ERROR_IS_FATAL ANY)
  file(REMOVE_RECURSE "${libtoolingSrc}")
endfunction()

# Main logic
if(EXISTS "${3rdPartyDir}/clang" OR EXISTS "${3rdPartyDir}/llvm")
  cmake_language(EXIT 0)
endif()

set(arFile "${scriptDir}/Libtooling.tzst")

file(DOWNLOAD
  https://github.com/${repo}/releases/download/libtooling-${v}/${os}-${arch}.tzst
  "${arFile}"
  SHOW_PROGRESS STATUS status
)
list(GET status 0 errCode)
list(GET status 1 err)

if(${errCode} EQUAL 0)
  file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
  cmake_language(EXIT 0)
endif()

# In Release mode or local development this shouldn't happen
if(${mode} STREQUAL Release OR NOT DEFINED ENV{CI})
  message(FATAL_ERROR "${err}")
endif()

getSrc()
build()
file(ARCHIVE_CREATE OUTPUT "${scriptDir}/${os}-${arch}.tzst" PATHS
"${3rdPartyDir}/clang" "${3rdPartyDir}/llvm" COMPRESSION Zstd COMPRESSION_LEVEL 19)
execute_process(COMMAND gh release upload libtooling-${v}
  "${scriptDir}/${os}-${arch}.tzst" -R ${repo} COMMAND_ERROR_IS_FATAL ANY)
