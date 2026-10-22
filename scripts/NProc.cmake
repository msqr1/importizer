# Portable way to get number of processes for the CLI

if(DEFINED CMAKE_SCRIPT_MODE_FILE)
  cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)

  # message() can't be used because it prints to stderr, not stdout
  execute_process(COMMAND "${CMAKE_COMMAND}"
    -E echo ${procCnt}
    COMMAND_ERROR_IS_FATAL ANY
  )
else()
  message(FATAL_ERROR "Don't use this file for inclusion!")
endif()
