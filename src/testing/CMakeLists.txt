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
usePCH(run-test)

add_custom_target(run-test-post-build ALL
  COMMAND "${CMAKE_COMMAND}"
    "-DSEARCH_DIRS=$<$<CONFIG:Debug>:${clangLibPath}>"
    "-DBIN_PATH=$<TARGET_FILE:importizer>"
    "-DPREDICTABLE_DIR=${CMAKE_BINARY_DIR}/importizer"
    -P "${scriptsDir}/PostBuild.cmake"
  COMMENT ""
  VERBATIM
)
add_dependencies(run-test-post-build run-test)
