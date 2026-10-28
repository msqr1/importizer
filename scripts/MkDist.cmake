# Make a distribution from a build directory, must be already built!

get_property(runMode GLOBAL PROPERTY CMAKE_ROLE)
if(NOT runMode STREQUAL "SCRIPT")
  message(FATAL_ERROR "File must run in cmake -P script mode.")
endif()

if(NOT DEFINED TYPE)
  message(FATAL_ERROR "Missing distribution type, specify one with -DTYPE=.")
endif()

if(NOT DEFINED BUILD_DIR)
  message(FATAL_ERROR "Missing build directory, specify one with -DBUILD_DIR=.")
endif()

if(NOT DEFINED OUT_DIR)
  message(FATAL_ERROR "Missing out directory, specify one with -DOUT_DIR=.")
endif()

set(knownTypes "Debug;Release")
if(NOT TYPE IN_LIST knownTypes)
  message(FATAL_ERROR "Unknown distribution type '${TYPE}', only Debug/Release is allowed.")
endif()
set(os $ENV{IMPORTIZER_OS})
set(arch $ENV{IMPORTIZER_ARCH})
set(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL ANY)

cmake_path(ABSOLUTE_PATH BUILD_DIR)
cmake_path(ABSOLUTE_PATH OUT_DIR)

execute_process(COMMAND "${CMAKE_COMMAND}"
  --install "${BUILD_DIR}"
  --config ${TYPE}
  --prefix "${OUT_DIR}/installation"
)

if(TYPE STREQUAL "Debug")
  file(ARCHIVE_CREATE
    OUTPUT "${OUT_DIR}/importizer-debug-${os}-${arch}.tar.zst"
    PATHS "${OUT_DIR}/installation"
    COMPRESSION Zstd
    COMPRESSION_LEVEL 19
  )
elseif(TYPE STREQUAL "Release")
  if(WIN32)
    set(binExtension ".exe")
  endif()
  file(COPY_FILE "${OUT_DIR}/installation/importizer${binExtension}"
    "${OUT_DIR}/importizer-release-${os}-${arch}${binExtension}")
endif()

file(REMOVE_RECURSE "${OUT_DIR}/installation")
