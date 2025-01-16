cmake_minimum_required(VERSION 3.25)
project(include2import)

if(CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(FATAL_ERROR "In-source build is unsupported")
endif()

# Setup the 3rdParty folder
execute_process(
  WORKING_DIRECTORY ../3rdParty 
  COMMAND ${CMAKE_COMMAND} -P Setup.cmake
)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_compile_options(-fsanitize=address,undefined,leak)
  add_link_options(-fsanitize=address,undefined,leak)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

add_subdirectory(src)

option(BUILD_TESTS "Build and run tests" OFF)
if(${BUILD_TESTS})
  add_subdirectory(test)
endif()