add_library(utils-lib
  Log.cc
  Glob.cc
  Toml.cc
)
require(utils-lib PRIVATE LLVM tomlc17)
target_link_libraries(utils-lib PRIVATE
  common-config
)
target_precompile_headers(utils-lib REUSE_FROM pch-LLVM)
