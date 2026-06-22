# Ensure that LibTooling (LLVM & Clang) is in 3rd-party/
## Uses Github CLI, needs GH_TOKEN env var

set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptsDir)
file(REAL_PATH "${scriptsDir}/.." root)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(v 22.1.8)
set(3rdPartyDir "${root}/3rd-party")
set(repo "msqr1/importizer")

if(IS_READABLE "${3rdPartyDir}/clang" OR IS_READABLE "${3rdPartyDir}/llvm")
  cmake_language(EXIT 0)
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
      set(refHash 28bdc8af61997c7af860eef65bc2c0d760bfac96f58ba4776397bdcb879f41c9)
    elseif(arch STREQUAL "arm64")
      set(refHash 4931fc068b72746104f072c93fceec3f361ce5309dcacd53a66642a4260b3f35)
    endif()
  elseif(os STREQUAL "macos")
    if(arch STREQUAL "amd64")
      set(refHash 2bec83a6d06222b862e60d8d2fc7ecb11ca3ae7a0ec25acca214cb51533162a7)
    elseif(arch STREQUAL "arm64")
      set(refHash 7d367caf1559f229976a963ae54baed3b596a5e30bcaa07781817d9c8bf65f83)
    endif()
  elseif(WIN32)
    if(arch STREQUAL "amd64")
      set(refHash 82810bbe9191958ce4e55dd2cef1ec480452b7bb679f19e30ffad9558413b5ca)
    elseif(arch STREQUAL "arm64")
      set(refHash 7e913982d99b30095e315b10904a080c8ea6693a764978b352b9f04e2ff13d8b)
    endif()
  endif()

  if(NOT hash STREQUAL refHash)
    file(REMOVE "${arFile}")
    message(FATAL_ERROR "Mismatched prebuilt LibTooling hash.")
  endif()
  file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${3rdPartyDir}")
  file(REMOVE "${arFile}")
  cmake_language(EXIT 0)
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

# Load our LibTooling preset
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/llvm/CMakeUserPresets.json" COPY_ON_ERROR)
file(CREATE_LINK "${root}/CMakePresets.json"
  "${llvmProjSrc}/clang/CMakeUserPresets.json" COPY_ON_ERROR)

include("${scriptsDir}/Exec.cmake")

# Build LibTooling
## First build LLVM
exec(${CMAKE_COMMAND}
  -S "${llvmProjSrc}/llvm"
  -B "${llvmProjSrc}/build"
  --preset libtooling-${os}
  --no-warn-unused-cli
)
exec(${CMAKE_COMMAND} --build "${llvmProjSrc}/build")
exec(${CMAKE_COMMAND}
  --install "${llvmProjSrc}/build"
  --prefix "${3rdPartyDir}/llvm"
  -j ${procCnt}
)

## Then build Clang
exec(${CMAKE_COMMAND}
  -S "${llvmProjSrc}/clang"
  -B "${llvmProjSrc}/build2"
  --preset libtooling-${os}
  "-DLLVM_DIR=${3rdPartyDir}/llvm/lib/cmake/llvm"
  --no-warn-unused-cli
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
