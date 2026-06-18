# Ensure that LibTooling (LLVM & Clang) is in 3rd-party/
## Uses Github CLI, needs GH_TOKEN env var

set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptDir)
file(REAL_PATH "${scriptDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(v 22.1.8)
set(3rdPartyDir "${root}/3rd-party")
set(repo "msqr1/importizer")
if(IS_READABLE "${3rdPartyDir}/clang" OR IS_READABLE "${3rdPartyDir}/llvm")
  cmake_language(EXIT 0)
endif()

if(WIN32)
  if(${arch} STREQUAL amd64)
    set(refHash)
  elseif(${arch} STREQUAL arm64)
    set(refHash)
  endif()
elseif(${os} STREQUAL macos)
  if(${arch} STREQUAL amd64)
    set(refHash)
  elseif(${arch} STREQUAL arm64)
    set(refHash)
  endif()
elseif(LINUX)
  if(${arch} STREQUAL amd64)
    set(refHash)
  elseif(${arch} STREQUAL arm64)
    set(refHash)
  endif()
endif()

set(arFile "${3rdPartyDir}/Libtooling.tzst")

file(DOWNLOAD
  https://github.com/${repo}/releases/download/libtooling-${v}/${os}-${arch}.tzst
  "${arFile}"
  STATUS status
)
list(GET status 0 ec)
list(GET status 1 err)

if(ec EQUAL 0)
  file(SHA256 "${arFile}" hash)
  if(NOT hash STREQUAL refHash)
    message(FATAL_ERROR "Mismatched prebuilt LibTooling hash.")
  endif()
  file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
  file(REMOVE "${arFile}")
  cmake_language(EXIT 0)
endif()

# In local development we don't want to build
if(NOT DEFINED ENV{CI})
  message(FATAL_ERROR "${err}")
endif()


file(MAKE_DIRECTORY "${3rdPartyDir}")
set(llvmProjSrc "${3rdPartyDir}/llvm-proj-src")

# Get LLVM Project source (which includes LLVM & Clang). There used to be standalone LLVM & Clang
# sources, but that seems to have ended in 21.x.x
set(arFile "${3rdPartyDir}/LlvmProj.tar.xz")
file(DOWNLOAD
  https://github.com/llvm/llvm-project/releases/download/llvmorg-${v}/llvm-project-${v}.src.tar.xz
  "${arFile}"
  EXPECTED_HASH SHA256=922f1817a0df7b1489272d18134ee0087a8b068828f87ac63b9861b1a9965888
)
file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
file(REMOVE "${arFile}")
file(RENAME "${3rdPartyDir}/llvm-project-${v}.src" "${llvmProjSrc}")

# Load our LibTooling preset
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/llvm/CMakeUserPresets.json" COPY_ON_ERROR)
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/clang/CMakeUserPresets.json" COPY_ON_ERROR)

# Build LibTooling
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
set(arFile "${scriptDir}/${os}-${arch}.tzst")
file(ARCHIVE_CREATE
  OUTPUT "${arFile}"
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
file(REMOVE "${arFile}")
