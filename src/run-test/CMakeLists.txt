add_executable(run-test Driver.cc CmpDir.cc)
require(run-test fmt WindowsHModular subprocess.h)
target_link_libraries(run-test PRIVATE
  common-config
  utils
)

# For the developer to be able to keep staying in the build directory
add_custom_command(POST_BUILD TARGET run-test
  COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_FILE:run-test>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:run-test>")
