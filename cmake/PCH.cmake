# Precompiled headers utilities
# For inclusion in CMakeLists.txt

include_guard(GLOBAL)

option(USE_PCH "Use precompiled headers" FALSE)
if(USE_PCH)
  add_library(PCHPkg OBJECT "${root}/src/Empty.cc")
  target_link_libraries(PCHPkg PRIVATE common-config)
  target_precompile_headers(PCHPkg PRIVATE
    <vector>
    <chrono>
  )
endif()
function(requirePCH target)
  if(USE_PCH)
    target_precompile_headers(${target} REUSE_FROM PCHPkg)
  endif()
endfunction()
