add_executable(run-test Driver.cc CmpDir.cc)
require(run-test PRIVATE LLVM)
target_link_libraries(run-test PRIVATE
  common-config
  utils-lib
  importizer-lib
)
target_precompile_headers(run-test REUSE_FROM pch-LLVM)

if(WIN32)
  add_custom_command(
    TARGET run-test
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -P "${scriptsDir}/PostBuild.cmake"
      "$<TARGET_FILE:run-test>"
      "${CMAKE_BINARY_DIR}/run-test"
      "${resourceDir}\\lib\\windows"
    VERBATIM
  )
else()
  add_custom_command(
    TARGET run-test
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -P "${scriptsDir}/PostBuild.cmake"
      "$<TARGET_FILE:run-test>"
      "${CMAKE_BINARY_DIR}/run-test"
    VERBATIM
  )
endif()
