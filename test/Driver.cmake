set(importizer ${CMAKE_ARGV3})
set(config ${CMAKE_ARGV4})
set(outDir ${CMAKE_ARGV5})
set(cmpDir ${CMAKE_ARGV6})
set(correctOutput ${CMAKE_ARGV7})
execute_process(RESULT_VARIABLE res COMMAND ${importizer} -c ${config} -o ${outDir})
if(NOT(${res} EQUAL 0))
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.29") 
    cmake_language(EXIT 1)
  else()
    message(FATAL_ERROR)
  endif()
endif()
execute_process(RESULT_VARIABLE res COMMAND ${cmpDir} ${outDir} ${correctOutput})
if(NOT(${res} EQUAL 0))
  if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.29") 
    cmake_language(EXIT 1)
  else()
    message(FATAL_ERROR)
  endif()
endif()