# Portable way to get number of process: $(cmake -P Nproc.cmake)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_LOGICAL_CORES)

# message() can't be used because it prints to stderr, not stdout
execute_process(COMMAND ${CMAKE_COMMAND} -E echo ${procCnt} COMMAND_ERROR_IS_FATAL ANY)
