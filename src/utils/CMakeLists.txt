add_library(utils FileOp.cc)
target_link_libraries(utils PRIVATE
  common-config
  fmt::fmt
)
