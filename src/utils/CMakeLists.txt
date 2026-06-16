add_library(utils Log.cc)
require(utils PRIVATE LLVM)
target_link_libraries(utils PRIVATE
  common-config
)
