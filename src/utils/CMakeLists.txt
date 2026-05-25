add_library(utils FileOp.cc Log.cc)
target_link_libraries(utils PRIVATE
  common-config
  fmt::fmt
)
