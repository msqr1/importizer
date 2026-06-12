add_executable(run-test Driver.cc CmpDir.cc)
require(run-test PRIVATE fmt)
target_link_libraries(run-test PRIVATE
  common-config
  utils
  importizer-lib
)

# More predictable output location
add_custom_command(POST_BUILD TARGET run-test
  COMMAND "${CMAKE_COMMAND}" -E create_hardlink "$<TARGET_FILE:run-test>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:run-test>")
