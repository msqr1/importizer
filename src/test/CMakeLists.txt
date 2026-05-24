CPMAddPackage("gh:sheredom/subprocess.h#main")
add_executable(test Driver.cc)

target_include_directories(test PRIVATE ${subprocess.h_SOURCE_DIR})
target_link_libraries(test PRIVATE
  common-config
  utils
  fmt::fmt
)
add_dependencies(test importizer)
enable_testing()
file(GLOB testDirs "./*")
foreach(testDir ${testDirs})
  if(NOT IS_DIRECTORY ${testDir})
    continue()
  endif()
  get_filename_component(testName ${testDir} NAME)
  add_test(
    NAME "${testName}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMAND "$<TARGET_FILE:test>"
      "$<TARGET_FILE:importizer>"
      "${testName}"
      "${CMAKE_BINARY_DIR}/test-outs/${testName}"
  )
endforeach()
