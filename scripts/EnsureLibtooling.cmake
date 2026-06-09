# Ensure that LibTooling (LLVM & Clang) is in 3rd-party/

set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptDir)
file(REAL_PATH "${scriptDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(v 22.1.6)
set(3rdPartyDir "${root}/3rd-party")
set(repo "msqr1/importizer")
if(IS_READABLE "${3rdPartyDir}/clang" OR IS_READABLE "${3rdPartyDir}/llvm")
  cmake_language(EXIT 0)
endif()

if(WIN32)
  if(${arch} STREQUAL x64)
    set(hash f464343c6f859f0f7e3ac47efa39ee54baf0ed7fbabc738222cfbb4dbcc9b9c9)
  elseif(${arch} STREQUAL arm64)
    set(hash 698db2ae8b0efcbdecb4ee9ec0d6b921ccf67a8cd3cc364b8e9430db6de0ae9b)
  endif()
elseif(${os} STREQUAL macos)
  if(${arch} STREQUAL x64)
    set(hash 0a6baaddb0c82496adc7133151807f18acda615f0a625c06c387e32c15bfe647)
  elseif(${arch} STREQUAL arm64)
    set(hash de38fb40ae84ded7362ef4c60e232983521374fe734004cbce17279d80515d3b)
  endif()
elseif(LINUX)
  if(${arch} STREQUAL x64)
  elseif(${arch} STREQUAL arm64)
  endif()
endif()

set(arFile "${scriptDir}/Libtooling.tzst")

file(DOWNLOAD
  https://github.com/${repo}/releases/download/libtooling-${v}/${os}-${arch}.tzst
  "${arFile}"
  EXPECTED_HASH "SHA256=${hash}"
  SHOW_PROGRESS STATUS status
)
list(GET status 0 errCode)
list(GET status 1 err)

if(${errCode} EQUAL 0)
  file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
  file(REMOVE "${arFile}")
  cmake_language(EXIT 0)
endif()

# In local development we don't want to build
if(NOT DEFINED ENV{CI})
  message(FATAL_ERROR "${err}")
endif()

set(llvmProjSrc "${scriptDir}/llvm-proj-src")
# Get LLVM Project source (which includes LLVM & Clang). There used to be standalone LLVM & Clang
# sources, but that seems to have ended in 21.x.x
set(arFile "${scriptDir}/LlvmProj.tar.xz")
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
file(REMOVE "${arFile}")
file(RENAME "${scriptDir}/llvm-project-${v}.src" "${llvmProjSrc}")

# Load our LibTooling preset
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/llvm/CMakeUserPresets.json" COPY_ON_ERROR)
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/clang/CMakeUserPresets.json" COPY_ON_ERROR)

# Build LibTooling
file(MAKE_DIRECTORY "${3rdPartyDir}")

## First build LLVM
execute_process(COMMAND ${CMAKE_COMMAND}
  -S "${llvmProjSrc}/llvm"
  -B "${llvmProjSrc}/build"
  --preset libtooling-${os}
  --no-warn-unused-cli
  COMMAND_ERROR_IS_FATAL ANY
)
execute_process(COMMAND ${CMAKE_COMMAND} --build "${llvmProjSrc}/build" COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND ${CMAKE_COMMAND}
  --install "${llvmProjSrc}/build"
  --prefix "${3rdPartyDir}/llvm"
  -j ${procCnt}
  COMMAND_ERROR_IS_FATAL ANY
)

## Then build Clang
execute_process(COMMAND ${CMAKE_COMMAND}
  -S "${llvmProjSrc}/clang"
  -B "${llvmProjSrc}/build2"
  --preset libtooling-${os}
  "-DLLVM_DIR=${3rdPartyDir}/llvm/lib/cmake/llvm"
  --no-warn-unused-cli
  COMMAND_ERROR_IS_FATAL ANY
)
execute_process(COMMAND ${CMAKE_COMMAND} --build "${llvmProjSrc}/build2" COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND ${CMAKE_COMMAND}
  --install "${llvmProjSrc}/build2"
  --prefix "${3rdPartyDir}/clang"
  -j ${procCnt}
  COMMAND_ERROR_IS_FATAL ANY
)

file(REMOVE_RECURSE "${llvmProjSrc}")

# Pack & upload
file(ARCHIVE_CREATE
  OUTPUT "${scriptDir}/${os}-${arch}.tzst"
  WORKING_DIRECTORY "${3rdPartyDir}"
  PATHS clang llvm
  COMPRESSION Zstd
  COMPRESSION_LEVEL 19
)
execute_process(COMMAND gh release upload libtooling-${v} "${scriptDir}/${os}-${arch}.tzst"
  --clobber
  -R ${repo}
  COMMAND_ERROR_IS_FATAL ANY
)
