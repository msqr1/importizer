add_library(utils FileOp.cc Log.cc)
require(utils fmt WindowsHModular)
target_link_libraries(utils PRIVATE
  common-config
)
