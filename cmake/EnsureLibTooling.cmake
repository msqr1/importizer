# Ensure a built LibTooling (LLVM and Clang) in 3rd-party/
# When on CI: Needs Github CLI and GH_TOKEN env var

get_property(runMode GLOBAL PROPERTY CMAKE_ROLE)
if(NOT runMode STREQUAL "PROJECT")
  message(FATAL_ERROR "File must run in project mode.")
endif()
include_guard(GLOBAL)

block(SCOPE_FOR VARIABLES PROPAGATE LLVM_DIR Clang_DIR)

set(v 23.1.5)
set(LLVM_DIR "${3rdPartyDir}/llvm/lib/cmake/llvm")
set(Clang_DIR "${3rdPartyDir}/clang/lib/cmake/clang")

if(IS_READABLE "${LLVM_DIR}" OR IS_READABLE "${Clang_DIR}")
  return()
endif()

set(arFile "${3rdPartyDir}/LibTooling.tar.zst")
set(repo "msqr1/importizer")

file(DOWNLOAD
  "https://github.com/${repo}/releases/download/libtooling-${v}/LibTooling-${os}-${arch}.tar.zst"
  "${arFile}"
  STATUS status
)
list(GET status 0 ec)
list(GET status 1 err)

if(ec EQUAL 0)
  file(SHA256 "${arFile}" hash)

  # Pre-built LibTooling hashes
  if(LINUX)
    if(arch STREQUAL "amd64")
      set(refHash 035eb261b5234adb05897c07e49bd607f4a9f165b25fb73f35df0e02d9eb92c2)
    elseif(arch STREQUAL "arm64")
      set(refHash 796bcf07afcf02eb2d7a9c094f8fd33c009b38295964b9b9c4d76e4d82f766c5)
    endif()
  elseif(APPLE)
    if(arch STREQUAL "amd64")
      set(refHash 9f783f93cc98d74737816ba934a92bb5c13374b7d19945255da2acf9e5a7f34d)
    elseif(arch STREQUAL "arm64")
      set(refHash 7544f5f7669f1f4b3466af4f27e993d72412f7418c5761dd77aca44c71a649c4)
    endif()
  elseif(WIN32)
    if(arch STREQUAL "amd64")
      set(refHash 93baa458bc628d1664ecddf8a0f3e207d9207dc63bf208c7c9b81e2a78a7f264)
    elseif(arch STREQUAL "arm64")
      set(refHash 5fb90086440144d81489acfe2ce893849588d548274c36fd46d3c929361d4eb5)
    endif()
  endif()

  if(NOT hash STREQUAL refHash)
    file(REMOVE "${arFile}")
    message(FATAL_ERROR "Mismatching pre-built LibTooling hash.")
  endif()
  file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
  file(REMOVE "${arFile}")
  return()
endif()

# Don't build on developer's device
if(NOT DEFINED ENV{CI})
  message(FATAL_ERROR "${err} (${ec})")
endif()

if(NOT DEFINED ENV{GH_TOKEN})
  message(FATAL_ERROR "Missing GH_TOKEN environment variable.")
endif()

find_program(ghCLI NAMES gh REQUIRED)

file(MAKE_DIRECTORY "${3rdPartyDir}")
set(LLVMProjSrc "${3rdPartyDir}/llvm-proj-src")

# Get LLVM Project source (which includes LLVM and Clang). There used to be standalone LLVM and
# Clang sources, but that seems to have ended in 21.x.x
set(arFile "${3rdPartyDir}/LLVMProj.tar.xz")
file(DOWNLOAD
  "https://github.com/llvm/llvm-project/releases/download/llvmorg-${v}/llvm-project-${v}.src.tar.xz"
  "${arFile}"
  EXPECTED_HASH SHA256=fb50b1eac93ddeb3aa8dab4cfe324dff74f4a9c2bdee6160c9e36188f8d06a6e
)
file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
file(REMOVE "${arFile}")
file(RENAME "${3rdPartyDir}/llvm-project-${v}.src" "${LLVMProjSrc}")

set(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL ANY)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)

# Build LibTooling
## Load toolchain, static and dynamic presets for LLVM
file(CREATE_LINK "${root}/CMakePresets.json"
  "${LLVMProjSrc}/llvm/CMakeUserPresets.json" SYMBOLIC)
file(CREATE_LINK "${root}/Toolchain.cmake"
  "${LLVMProjSrc}/llvm/Toolchain.cmake" SYMBOLIC)
file(CREATE_LINK "${root}/DynPresets.cmake"
  "${LLVMProjSrc}/llvm/DynPresets.cmake" SYMBOLIC)

## Build LLVM
execute_process(COMMAND "${CMAKE_COMMAND}"
  -S "${LLVMProjSrc}/llvm"
  -B "${LLVMProjSrc}/build"
  --preset llvm
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${LLVMProjSrc}/build")
execute_process(COMMAND "${CMAKE_COMMAND}"
  --install "${LLVMProjSrc}/build"
  --prefix "${3rdPartyDir}/llvm"
  -j "${procCnt}"
)

## Load Clang's presets (same as LLVM)
file(CREATE_LINK "${root}/CMakePresets.json"
  "${LLVMProjSrc}/clang/CMakeUserPresets.json" SYMBOLIC)
file(CREATE_LINK "${root}/Toolchain.cmake"
  "${LLVMProjSrc}/clang/Toolchain.cmake" SYMBOLIC)
file(CREATE_LINK "${root}/DynPresets.cmake"
  "${LLVMProjSrc}/clang/DynPresets.cmake" SYMBOLIC)

## Build Clang
execute_process(COMMAND "${CMAKE_COMMAND}"
  -S "${LLVMProjSrc}/clang"
  -B "${LLVMProjSrc}/build2"
  --preset clang
  "-DLLVM_DIR=${3rdPartyDir}/llvm/lib/cmake/llvm"
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${LLVMProjSrc}/build2")
execute_process(COMMAND "${CMAKE_COMMAND}"
  --install "${LLVMProjSrc}/build2"
  --prefix "${3rdPartyDir}/clang"
  -j "${procCnt}"
)

file(REMOVE_RECURSE "${LLVMProjSrc}")

# Pack and upload
set(arFile "${3rdPartyDir}/LibTooling-${os}-${arch}.tar.zst")
file(ARCHIVE_CREATE
  OUTPUT "${arFile}"
  WORKING_DIRECTORY "${3rdPartyDir}"
  PATHS clang llvm
  COMPRESSION Zstd
  COMPRESSION_LEVEL 19
)
execute_process(COMMAND gh release upload "libtooling-${v}" "${arFile}" --clobber -R "${repo}")
file(REMOVE "${arFile}")

endblock()
