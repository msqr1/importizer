# Ensure that LibTooling (LLVM & Clang) is in 3rd-party/
# Can be used either as script or included
## Uses Github CLI, needs GH_TOKEN env var

block(SCOPE_FOR VARIABLES PROPAGATE LLVM_DIR Clang_DIR)


if(DEFINED CMAKE_SCRIPT_MODE_FILE)
  if(LINUX)
    set(os "linux")
  elseif(APPLE)
    set(os "macos")
  elseif(WIN32)
    set(os "windows")
  endif()
  set(arch $ENV{IMPORTIZER_ARCH})
  cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptsDir)
  file(REAL_PATH "${scriptsDir}/.." root)
  set(3rdPartyDir "${root}/3rd-party")
endif()

cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(v 22.1.8)
set(repo "msqr1/importizer")
set(LLVM_DIR "${3rdPartyDir}/llvm/lib/cmake/llvm")
set(Clang_DIR "${3rdPartyDir}/clang/lib/cmake/clang")

if(IS_READABLE "${3rdPartyDir}/clang" OR IS_READABLE "${3rdPartyDir}/llvm")
  return()
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

  # Prebuilt LibTooling hashes
  if(LINUX)
    if(arch STREQUAL "amd64")
    elseif(arch STREQUAL "arm64")
    endif()
  elseif(APPLE)
    if(arch STREQUAL "amd64")
      set(refHash)
    elseif(arch STREQUAL "arm64")
      set(refHash)
    endif()
  elseif(WIN32)
    if(arch STREQUAL "amd64")
      set(refHash)
    elseif(arch STREQUAL "arm64")
      set(refHash eb693f9616afb2e7171aceb47d9eac8e5769a6643933e9fc928f5f52116cce07)
    endif()
  endif()

  if(NOT hash STREQUAL refHash)
    file(REMOVE "${arFile}")
    message(FATAL_ERROR "Mismatched prebuilt LibTooling hash.")
  endif()
  file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
  file(REMOVE "${arFile}")
  return()
endif()

# Don't build on developer's device
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

include("${scriptsDir}/Exec.cmake")

# Build LibTooling
## Load toolchain, static & dynamic presets for LLVM
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/llvm/CMakeUserPresets.json" COPY_ON_ERROR SYMBOLIC)
file(CREATE_LINK "${root}/Toolchain.cmake"
  "${llvmProjSrc}/llvm/Toolchain.cmake" COPY_ON_ERROR SYMBOLIC)
file(CREATE_LINK "${root}/DynPresets.cmake"
  "${llvmProjSrc}/llvm/DynPresets.cmake" COPY_ON_ERROR SYMBOLIC)

## Build LLVM
exec(${CMAKE_COMMAND}
  -S "${llvmProjSrc}/llvm"
  -B "${llvmProjSrc}/build"
  --preset llvm
)
exec(${CMAKE_COMMAND} --build "${llvmProjSrc}/build")
exec(${CMAKE_COMMAND}
  --install "${llvmProjSrc}/build"
  --prefix "${3rdPartyDir}/llvm"
  -j ${procCnt}
)

## Load Clang's presets (same as LLVM)
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/clang/CMakeUserPresets.json" COPY_ON_ERROR SYMBOLIC)
file(CREATE_LINK "${root}/Toolchain.cmake"
  "${llvmProjSrc}/clang/Toolchain.cmake" COPY_ON_ERROR SYMBOLIC)
file(CREATE_LINK "${root}/DynPresets.cmake"
  "${llvmProjSrc}/clang/DynPresets.cmake" COPY_ON_ERROR SYMBOLIC)

## Build Clang
exec(${CMAKE_COMMAND}
  -S "${llvmProjSrc}/clang"
  -B "${llvmProjSrc}/build2"
  --preset clang
  "-DLLVM_DIR=${3rdPartyDir}/llvm/lib/cmake/llvm"
)
exec(${CMAKE_COMMAND} --build "${llvmProjSrc}/build2")
exec(${CMAKE_COMMAND}
  --install "${llvmProjSrc}/build2"
  --prefix "${3rdPartyDir}/clang"
  -j ${procCnt}
)

file(REMOVE_RECURSE "${llvmProjSrc}")

# Pack & upload
set(arFile "${3rdPartyDir}/${os}-${arch}.tzst")
file(ARCHIVE_CREATE
  OUTPUT "${arFile}"
  WORKING_DIRECTORY "${3rdPartyDir}"
  PATHS clang llvm
  COMPRESSION Zstd
  COMPRESSION_LEVEL 19
)
exec(gh release upload libtooling-${v} "${arFile}" --clobber -R ${repo})
file(REMOVE "${arFile}")

endblock()
