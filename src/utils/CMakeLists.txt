add_library(utils Log.cc)
require(utils PRIVATE LLVM)
requirePch(utils LLVM)
target_link_libraries(utils PRIVATE
  common-config
)
