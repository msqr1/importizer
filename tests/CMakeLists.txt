enable_testing()

# Unit tests
set(unitTestSuites "Diag;Glob;FS;Opts")
foreach(unitTestName ${unitTestSuites})
  add_test(
    NAME ${unitTestName}
    COMMAND "$<TARGET_FILE:run-test>"
      unit
      ${unitTestName}
  )
endforeach()
set_tests_properties(${unitTestSuites} PROPERTIES LABELS "unit")

# End-to-end tests
set(E2ETestNames)
file(GLOB testDirs ./*)
foreach(testDir ${testDirs})
  if(NOT IS_DIRECTORY "${testDir}")
    continue()
  endif()
  cmake_path(GET testDir FILENAME E2ETestName)
  add_test(
    NAME "${E2ETestName}"
    COMMAND "$<TARGET_FILE:run-test>"
      E2E
      "${CMAKE_CURRENT_SOURCE_DIR}/${E2ETestName}"
      "${CMAKE_CURRENT_BINARY_DIR}/${E2ETestName}"
  )
  list(APPEND E2ETestNames ${E2ETestName})
endforeach()
set_tests_properties(${E2ETestNames} PROPERTIES LABELS "E2E")
