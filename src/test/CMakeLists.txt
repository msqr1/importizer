CPMAddPackage("gh:sheredom/subprocess.h#main")
add_executable(test Driver.cc)

target_include_directories(test PRIVATE ${subprocess.h_SOURCE_DIR})
target_link_libraries(test PRIVATE
  common-config
  utils
  fmt::fmt
)
