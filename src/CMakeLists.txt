add_executable(importizer Util.cc Regex.cc OptProcessor.cc FileOp.cc Driver.cc 
  Preprocessor.cc Preamble.cc Directive.cc Minimizer.cc)
target_compile_features(importizer PRIVATE cxx_std_23)
option(PCH "Utilize precompiled headers" OFF)
if(PCH)

  # CMake doesn't realize these are also required when going with PCH,
  # and there is no way to add it just in the PCH, so I just do it here
  target_compile_definitions(importizer PRIVATE TOML_EXCEPTIONS=0)
  target_compile_definitions(importizer PRIVATE TOML_ENABLE_FORMATTERS=0)
  target_compile_definitions(importizer PRIVATE ARGS_NOEXCEPT)
  target_compile_definitions(importizer PRIVATE PCRE2_CODE_UNIT_WIDTH=8)

  # 3rd-party headers rarely changes
  set(3rdPartyHeaders args.hpp toml++/include/toml++/toml.hpp fmt/include/fmt/format.h
  fmt/include/fmt/base.h fmt/include/fmt/std.h pcre2/src/pcre2.h.generic)
  list(TRANSFORM 3rdPartyHeaders PREPEND ../3rdParty/)

  # Project headers that should not be changed frequently
  set(stableHeaders Util.hpp Regex.hpp FileOp.hpp)
  foreach(header IN LISTS 3rdPartyHeaders stableHeaders)
    target_precompile_headers(importizer PRIVATE ${header})
  endforeach()
endif()

add_subdirectory(../3rdParty/fmt "${CMAKE_BINARY_DIR}/fmt")
target_link_libraries(importizer fmt::fmt)
add_subdirectory(../3rdParty/toml++ "${CMAKE_BINARY_DIR}/toml++")
target_link_libraries(importizer tomlplusplus::tomlplusplus)
set(PCRE2_BUILD_TESTS off)
set(PCRE2_BUILD_PCRE2GREP off)
set(PCRE2_SUPPORT_JIT on CACHE BOOL "PCRE2 JIT")

# Another fix for PCRE2, it uses option() for default values with CMAKE < 3.12, so it
# overrides my variables above. For this policy we can't cmake_policy()
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(../3rdParty/pcre2 "${CMAKE_BINARY_DIR}/pcre2")
target_link_libraries(importizer pcre2-8)

# For actions/upload-artifact
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:importizer>" 
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")