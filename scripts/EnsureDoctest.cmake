# Ensure a built doctest 3rd-party/
# Can be used either as script or included
## Uses Github CLI, needs GH_TOKEN env var

include_guard(GLOBAL)

block(SCOPE_FOR VARIABLES PROPAGATE doctest_DIR)

set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
if(DEFINED CMAKE_SCRIPT_MODE_FILE)
  cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH scriptsDir)
  file(REAL_PATH "${scriptsDir}/.." root)
  set(3rdPartyDir "${root}/3rd-party")
endif()

cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)
set(v 2.5.3)
set(doctest_DIR "${3rdPartyDir}/doctest/lib/cmake/doctest")

if(IS_READABLE "${doctest_DIR}")
  return()
endif()

set(doctestSrc "${3rdPartyDir}/doctest-src")
file(MAKE_DIRECTORY "${doctestSrc}")

# Get doctest source
set(arFile "${3rdPartyDir}/Doctest.tar.gz")
file(DOWNLOAD
  https://github.com/doctest/doctest/releases/download/v${v}/doctest-v${v}.tar.gz
  "${arFile}"
  EXPECTED_HASH SHA256=eb4a39729d091e43d4d593888bf4d145ee9a0f0e13f7404484b6f2fb60d5d50f
)
file(ARCHIVE_EXTRACT INPUT "${arFile}" DESTINATION "${doctestSrc}")
file(REMOVE "${arFile}")

# Load toolchain, static and dynamic presets
file(CREATE_LINK "${root}/CMakePresets.json"
  "${doctestSrc}/CMakeUserPresets.json" SYMBOLIC)
file(CREATE_LINK "${root}/Toolchain.cmake"
  "${doctestSrc}/Toolchain.cmake" SYMBOLIC)
file(CREATE_LINK "${root}/DynPresets.cmake"
  "${doctestSrc}/DynPresets.cmake" SYMBOLIC)

include("${scriptsDir}/Exec.cmake")

# Build
exec("${CMAKE_COMMAND}"
  -S "${doctestSrc}"
  -B "${doctestSrc}/build"
  --preset doctest
)
exec("${CMAKE_COMMAND}" --build "${doctestSrc}/build")
exec("${CMAKE_COMMAND}"
  --install "${doctestSrc}/build"
  --prefix "${3rdPartyDir}/doctest"
  -j ${procCnt}
)

file(REMOVE_RECURSE "${doctestSrc}")

endblock()
