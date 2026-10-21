add_executable(run-test
  Driver.cc
  E2E.cc
  Unit.cc
  DiagTest.cc
  FSTest.cc
  GlobTest.cc
  OptsTest.cc
)
require(run-test PRIVATE LLVM doctest)
target_link_libraries(run-test PRIVATE
  common-config
  utils-lib
  importizer-lib
)
target_precompile_headers(run-test REUSE_FROM pch-LLVM)

add_custom_command(
  TARGET run-test
  POST_BUILD
  COMMAND "${CMAKE_COMMAND}"
    "-DsearchDirs=${clangLibPath}"
    "-Dexe=$<TARGET_FILE:run-test>"
    "-DpredictableDir=${CMAKE_BINARY_DIR}/run-test"
    -P "${scriptsDir}/PostBuild.cmake"
  VERBATIM
)

foreach(suiteName "Diag;Glob;FS;Opts")
  add_test(
    NAME "${suiteName}"
    COMMAND "$<TARGET_FILE:run-test>"
      "unit"
      "${suiteName}"
  )
endforeach()
