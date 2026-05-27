add_executable(test Driver.cc CmpDir.cc)
require(test fmt WindowsHModular subprocess.h)
target_link_libraries(test PRIVATE
  common-config
  utils
)

# For the developer to be able to keep staying in the build directory
add_custom_command(POST_BUILD TARGET test
  COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_FILE:test>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:test>")
