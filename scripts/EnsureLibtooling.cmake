# Ensure that LibTooling (LLVM & Clang) is in 3rd-party/

set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptDir)
file(REAL_PATH "${scriptDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(v 22.1.6)
set(3rdPartyDir "${root}/3rd-party")
set(repo "msqr1/importizer")
#[[
if(EXISTS "${3rdPartyDir}/clang" OR EXISTS "${3rdPartyDir}/llvm")
  cmake_language(EXIT 0)
endif()

if(WIN32)
  if(${arch} STREQUAL x64)
    set(hash 775b4ff98c2c16ccf78b3e1a6ea299f322222beb81986cb228c75c4f094b0d17)
  elseif(${arch} STREQUAL arm64)
    set(hash 5b243e4e288c5894877300c05cf29f45e1f4c90170de2f62a200712faabfbb79)
  endif()
elseif(${os} STREQUAL macos)
  if(${arch} STREQUAL x64)
    set(hash 578c01259b1cc4502814cb74eb2152280ac9940e1b1a8f97308be79961c4e83a)
  elseif(${arch} STREQUAL arm64)
    set(hash 9d585bfe613480b3aadf0620eab8ca8e774e70486240dac3fc4ba6cf0311778e)
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
  cmake_language(EXIT 0)
endif()

# In local development we don't want to build
if(NOT DEFINED ENV{CI})
  message(FATAL_ERROR "${err}")
endif()

set(llvmProjSrc "${scriptDir}/llvm-proj-src")

# Get LLVM Project source (which includes LLVM and Clang)
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
file(RENAME "${scriptDir}/llvm-project-${v}.src" "${llvmProjSrc}")

# Load our LibTooling preset
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/llvm/CMakeUserPresets.json" COPY_ON_ERROR)
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/clang/CMakeUserPresets.json" COPY_ON_ERROR)
file(REMOVE "${arFile}")

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
execute_process(COMMAND ${CMAKE_COMMAND} -S "${llvmProjSrc}/clang"
  -B "${llvmProjSrc}/build2"
  --preset libtooling-windows
  -DLLVM_DIR="${3rdPartyDir}/llvm/lib/cmake/llvm"
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
#]]
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
  COMMAND_ERROR_IS_FATAL ANY)
