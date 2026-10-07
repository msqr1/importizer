add_library(utils-lib
  Fs.cc
  Log.cc
  Glob.cc
)
require(utils-lib PRIVATE LLVM)
target_link_libraries(utils-lib PRIVATE
  common-config
)
target_precompile_headers(utils-lib REUSE_FROM pch-LLVM)
