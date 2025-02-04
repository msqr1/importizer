cmake_minimum_required(VERSION 3.25)

function(gitClone repo hash outDir)
  file(MAKE_DIRECTORY ${outDir})
  execute_process(
    WORKING_DIRECTORY ${outDir} 
    COMMAND git init
  )
  execute_process(
    WORKING_DIRECTORY ${outDir} 
    COMMAND git fetch --depth 1 "https://github.com/${repo}" ${hash}
  )
  execute_process(
    WORKING_DIRECTORY ${outDir} 
    COMMAND git -c advice.detachedHead=false checkout ${hash}
  )
  execute_process(
    WORKING_DIRECTORY ${outDir}
    COMMAND git submodule update --init --recursive --depth 1
  )
endfunction(gitClone repo hash outDir)

# 11.1.2
if(NOT EXISTS fmt)
  gitClone(fmtlib/fmt 8303d140a1a11f19b982a9f664bbe59a1ccda3f4 fmt)
endif()

# 3.4.0
if(NOT EXISTS toml++)
  gitClone(marzer/tomlplusplus 30172438cee64926dc41fdd9c11fb3ba5b2ba9de toml++)
endif()

# 10.45-RC1
if(NOT EXISTS pcre2)
  gitClone(PCRE2Project/pcre2 e2985c439b7fa374b4bd3052ca413e29ed1ad590 pcre2)
endif()

if(NOT EXISTS Argparse.hpp)
  file(DOWNLOAD https://raw.githubusercontent.com/p-ranav/argparse/refs/tags/v3.1/include/argparse/argparse.hpp Argparse.hpp)
endif()