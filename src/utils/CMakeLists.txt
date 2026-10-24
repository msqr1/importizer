add_library(utils-lib
  FS.cc
  Diag.cc
  Glob.cc
)
require(utils-lib PRIVATE LLVM)
target_link_libraries(utils-lib PRIVATE
  common-config
)
usePCH(utils-lib)
