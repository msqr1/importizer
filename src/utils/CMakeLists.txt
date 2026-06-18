add_library(utils-lib Log.cc)
require(utils-lib PRIVATE LLVM)
target_link_libraries(utils-lib PRIVATE
  common-config
)
target_precompile_headers(utils-lib REUSE_FROM pch-LLVM)
