
add_executable(importizer Base.cc Regex.cc OptProcessor.cc FileOp.cc Main.cc 
  Preprocessor.cc Preamble.cc Directive.cc CondMinimizer.cc)

# You won't use this name anymore, so just name it anything, I chose "q"
target_sources(importizer PRIVATE FILE_SET q TYPE CXX_MODULES FILES Base.ixx Regex.ixx 
  OptProcessor.ixx FileOp.ixx Preprocessor.ixx Preamble.ixx Directive.ixx
  CondMinimizer.ixx)
target_compile_features(importizer PRIVATE cxx_std_23)

# Removed precompiled headers

add_subdirectory(../3rdParty/fmt "${CMAKE_BINARY_DIR}/fmt")
target_link_libraries(importizer fmt::fmt)
target_compile_definitions(importizer PRIVATE TOML_EXCEPTIONS=0)
target_compile_definitions(importizer PRIVATE TOML_ENABLE_FORMATTERS=0)
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
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:importizer> 
  ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>)