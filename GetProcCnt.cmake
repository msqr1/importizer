cmake_host_system_information(RESULT procCnt QUERY NUMBER_OF_PHYSICAL_CORES)
message(${procCnt})