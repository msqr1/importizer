# Ensure a built LibTooling (LLVM and Clang) in 3rd-party/
# When on CI: Needs Github CLI and GH_TOKEN env var

get_property(runMode GLOBAL PROPERTY CMAKE_ROLE)
if(NOT runMode STREQUAL "PROJECT")
  message(FATAL_ERROR "File must run in project mode.")
endif()
include_guard(GLOBAL)

block(SCOPE_FOR VARIABLES PROPAGATE LLVM_DIR Clang_DIR)

set(v 22.1.8)
set(LLVM_DIR "${3rdPartyDir}/llvm/lib/cmake/llvm")
set(Clang_DIR "${3rdPartyDir}/clang/lib/cmake/clang")

if(IS_READABLE "${LLVM_DIR}" OR IS_READABLE "${Clang_DIR}")
  return()
endif()

set(arFile "${3rdPartyDir}/LibTooling.tar.zst")
set(repo "msqr1/importizer")

file(DOWNLOAD
  https://github.com/${repo}/releases/download/libtooling-${v}/LibTooling-${os}-${arch}.tar.zst
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
      set(refHash 8bf900ed8505eb2ea85008b9a25682c30ddfafdfdd42d8c25b6249603f431ec4)
    elseif(arch STREQUAL "arm64")
      set(refHash 4263b3039e33aff3873dd97fb769c751b53277cf348461b64ed24fc7b49f65ae)
    endif()
  elseif(APPLE)
    if(arch STREQUAL "amd64")
      set(refHash d1eb7ec0a39239e97f82c74c23ef5a2894bf42010faccd96e616cff761fb5382)
    elseif(arch STREQUAL "arm64")
      set(refHash b908b54460b1ec02d445c15038d3596bad9ea70523327743a3da0960616c68f3)
    endif()
  elseif(WIN32)
    if(arch STREQUAL "amd64")
      set(refHash d9d2c3885bd2a0a0e2c37b8853508fc032d50b4e36b99c858915ef8c02b34a31)
    elseif(arch STREQUAL "arm64")
      set(refHash af002e746b0aabeea00cd1fef5a0daf1463de8ee53c6b78e0b03b5806f9fefbf)
    endif()
  endif()

  if(NOT hash STREQUAL refHash)
    file(REMOVE "${arFile}")
    message(FATAL_ERROR "Mismatched pre-built LibTooling hash.")
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
  https://github.com/llvm/llvm-project/releases/download/llvmorg-${v}/llvm-project-${v}.src.tar.xz
  "${arFile}"
  EXPECTED_HASH SHA256=922f1817a0df7b1489272d18134ee0087a8b068828f87ac63b9861b1a9965888
)
file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
file(REMOVE "${arFile}")
file(RENAME "${3rdPartyDir}/llvm-project-${v}.src" "${LLVMProjSrc}")

set(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL ANY)

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
  -j ${procCnt}
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
  -j ${procCnt}
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
execute_process(COMMAND gh release upload libtooling-${v} "${arFile}" --clobber -R ${repo})
file(REMOVE "${arFile}")

endblock()
