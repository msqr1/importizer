add_executable(run-test Driver.cc CmpDir.cc)
require(run-test PRIVATE LLVM)
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
