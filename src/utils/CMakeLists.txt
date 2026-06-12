add_library(utils FileOp.cc Log.cc)
require(utils PRIVATE fmt WindowsHModular)
target_link_libraries(utils PRIVATE
  common-config
)
