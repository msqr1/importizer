enable_testing()

# Add unit tests
foreach(suiteName "Diag;Glob;FS;Opts")
  add_test(
    NAME "${suiteName}"
    LABELS unit
    COMMAND "$<TARGET_FILE:run-test>"
      unit
      "${suiteName}"
  )
endforeach()

# Add end-to-end tests
file(GLOB testDirs ./*)
foreach(testDir ${testDirs})
  if(NOT IS_DIRECTORY "${testDir}")
    continue()
  endif()
  cmake_path(GET testDir FILENAME testName)
  add_test(
    NAME "${testName}"
    LABELS E2E
    COMMAND "$<TARGET_FILE:run-test>"
      E2E
      "${CMAKE_CURRENT_SOURCE_DIR}/${testName}"
      "${CMAKE_CURRENT_BINARY_DIR}/${testName}"
  )
endforeach()
