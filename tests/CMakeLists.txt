enable_testing()

# Unit tests
set(unitTestSuites "FS;Glob;Config;")
foreach(unitTestSuite ${unitTestSuites})
  add_test(
    NAME ${unitTestSuite}
    COMMAND "$<TARGET_FILE:test-tool>"
      run-unit
      ${unitTestSuite}
  )
endforeach()
set_tests_properties(${unitTestSuites} PROPERTIES LABELS "unit")

# End-to-end tests
set(e2eTestNames)
file(GLOB testDirs ./*)
foreach(testDir ${testDirs})
  if(NOT IS_DIRECTORY "${testDir}")
    continue()
  endif()
  cmake_path(GET testDir FILENAME e2eTestName)
  add_test(
    NAME "${e2eTestName}"
    COMMAND "$<TARGET_FILE:test-tool>"
      run-e2e
      "${CMAKE_CURRENT_SOURCE_DIR}/${e2eTestName}"
      "${CMAKE_CURRENT_BINARY_DIR}/${e2eTestName}"
  )
  list(APPEND e2eTestNames ${e2eTestName})
endforeach()
set_tests_properties(${e2eTestNames} PROPERTIES LABELS "E2E")
