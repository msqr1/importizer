add_executable(test-tool
  Driver.cc
  E2E.cc
  Unit.cc
  MemFS.cc
  FSTest.cc
  GlobTest.cc
  OptsTest.cc
)
require(test-tool PRIVATE LLVM doctest)
target_link_libraries(test-tool PRIVATE
  common-config
  utils-lib
  importizer-lib
)
usePCH(test-tool)

add_custom_target(test-tool-post-build ALL
  COMMAND "${CMAKE_COMMAND}"
    "-DSEARCH_DIRS=$<$<CONFIG:Debug>:${clangLibPath}>"
    "-DBIN_PATH=$<TARGET_FILE:test-tool>"
    "-DPREDICTABLE_DIR=${CMAKE_BINARY_DIR}/testing"
    -P "${scriptsDir}/PostBuild.cmake"
  COMMENT ""
  VERBATIM
)
add_dependencies(test-tool-post-build test-tool)
