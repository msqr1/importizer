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
requirePCH(importizer)

add_custom_command(
  TARGET run-test
  POST_BUILD
  COMMAND "${CMAKE_COMMAND}"
    "-DSEARCH_DIRS=${clangLibPath}"
    "-DBIN_PATH=$<TARGET_FILE:run-test>"
    "-DPREDICTABLE_DIR=${CMAKE_BINARY_DIR}/testing"
    -P "${scriptsDir}/PostBuild.cmake"
  VERBATIM
)
