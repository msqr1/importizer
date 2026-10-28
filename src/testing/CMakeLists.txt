add_executable(run-test
  Driver.cc
  E2E.cc
  Unit.cc
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
usePCH(run-test)

add_custom_target(run-test-post-build ALL
  COMMAND "${CMAKE_COMMAND}"
    "-DSEARCH_DIRS=$<$<CONFIG:Debug>:${clangLibPath}>"
    "-DBIN_PATH=$<TARGET_FILE:run-test>"
    "-DPREDICTABLE_DIR=${CMAKE_BINARY_DIR}/testing"
    -P "${scriptsDir}/PostBuild.cmake"
  COMMENT ""
  VERBATIM
)
add_dependencies(run-test-post-build run-test)
