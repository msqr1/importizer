if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

string(COMPARE EQUAL "${CMAKE_BULD_TYPE}" "Release" releaseBuild)
if(releaseBuild)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_executable(executable Base.cc Regex.cc ArgProcessor.cc FileOp.cc Main.cc 
  Preprocessor.cc Preamble.cc Directive.cc StdInclude.cc)
target_compile_features(executable PRIVATE cxx_std_23)
target_compile_options(executable PRIVATE -Wall -Wextra)

# 3rd-party headers rarely changes
set(3rdPartyHeaders Argparse.hpp toml++/include/toml++/toml.hpp fmt/include/fmt/format.h)
list(TRANSFORM 3rdPartyHeaders PREPEND "../3rdParty/")

# Project headers that should not be changed frequently
set(stableHeaders Base.hpp Regex.hpp StdInclude.hpp)
foreach(header IN LISTS 3rdPartyHeaders stableHeaders)
  target_precompile_headers(executable PRIVATE ${header})
endforeach()

# For some reason dumb reasons, PCRE2 setting a cached BUILD_SHARED_LIBS like so:
# SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries.")
# will override my set(BUILD_SHARED_LIBS on) when building the first time because normal
# variables should override cached ones. In this case, it just doesn't work???
set(BUILD_SHARED_LIBS on CACHE BOOL "Build shared libraries.")

add_subdirectory(../3rdParty/fmt "${CMAKE_BINARY_DIR}/fmt")
target_link_libraries(executable fmt::fmt)

target_compile_definitions(executable PRIVATE TOML_EXCEPTIONS=0)
target_compile_definitions(executable PRIVATE TOML_ENABLE_FORMATTERS=0)
add_subdirectory(../3rdParty/toml++ "${CMAKE_BINARY_DIR}/toml++")
target_link_libraries(executable tomlplusplus::tomlplusplus)

set(PCRE2_BUILD_TESTS off)
set(PCRE2_BUILD_PCRE2GREP off)
set(BUILD_STATIC_LIBS off)
set(PCRE2_SUPPORT_JIT on CACHE BOOL "PCRE2 JIT")

# Another fix for PCRE2, it uses option() for default values with CMAKE < 3.12, so it just
# override my variables above
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
add_subdirectory(../3rdParty/pcre2 "${CMAKE_BINARY_DIR}/pcre2")
target_link_libraries(executable pcre2-8)