enable_testing()
file(GLOB testDirs ./*)
foreach(testDir ${testDirs})
  if(NOT IS_DIRECTORY ${testDir})
    continue()
  endif()
  get_filename_component(testName ${testDir} NAME)
  add_test(
    NAME "${testName}"
    COMMAND "$<TARGET_FILE:run-test>"
      "${CMAKE_CURRENT_SOURCE_DIR}/${testName}"
      "${CMAKE_CURRENT_BINARY_DIR}/${testName}"
  )
endforeach()
