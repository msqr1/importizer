add_library(utils Log.cc)
require(utils PRIVATE LLVM)
target_link_libraries(utils PRIVATE
  common-config
)
target_precompile_headers(utils REUSE_FROM pch-LLVM)
