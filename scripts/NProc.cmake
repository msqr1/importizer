# Portable way to get number of processes for the CLI

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
  message(FATAL_ERROR "File must be run as CMake script.")
endif()

cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)

# message() can't be used because it prints to stderr, not stdout
execute_process(COMMAND "${CMAKE_COMMAND}"
  -E echo ${procCnt}
  COMMAND_ERROR_IS_FATAL ANY
)
