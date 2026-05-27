# Portable nproc, use $(cmake -P nproc.cmake) instead of $(nproc)
cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_PHYSICAL_CORES)

# message() can't be used because it prints to stderr, not stdout
execute_process(COMMAND ${CMAKE_COMMAND} -E echo ${procCnt})
