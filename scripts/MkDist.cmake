# Make a distribution from a build directory, must be already built!

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
  message(FATAL_ERROR "File must be run as CMake script.")
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
    OUTPUT "${OUT_DIR}/importizer-${os}-${arch}.tar.zst"
    PATHS "${OUT_DIR}/installation"
    COMPRESSION Zstd
    COMPRESSION_LEVEL 19
  )
elseif(TYPE STREQUAL "Release")
  set(bin "importizer")
  if(WIN32)
    set(bin "${bin}.exe")
  endif()
  file(COPY_FILE "${OUT_DIR}/installation/${bin}" "${OUT_DIR}/${bin}")
endif()

file(REMOVE_RECURSE "${OUT_DIR}/installation")
