cmake_minimum_required(VERSION 3.25)

function(gitClone repo hash outDir)
  string(CONCAT url "https://github.com/" ${repo})
  file(MAKE_DIRECTORY ${outDir})
  execute_process(
    WORKING_DIRECTORY ${outDir} 
    COMMAND git init
  )
  execute_process(
    WORKING_DIRECTORY ${outDir} 
    COMMAND git fetch --depth 1 ${url} ${hash}
  )
  execute_process(
    WORKING_DIRECTORY ${outDir} 
    COMMAND git checkout ${hash}
  )
endfunction(gitClone repo hash outDir)

if(NOT EXISTS fmt)
  gitClone(fmtlib/fmt 0c9fce2ffefecfdce794e1859584e25877b7b592 fmt)
endif()

if(NOT EXISTS toml++)
  gitClone(marzer/tomlplusplus 30172438cee64926dc41fdd9c11fb3ba5b2ba9de toml++)
endif()

if(NOT EXISTS pcre2)
  gitClone(PCRE2Project/pcre2 6ae58beca071f13ccfed31d03b3f479ab520639b pcre2)
endif()

if(NOT EXISTS Catch2)
  gitClone(catchorg/Catch2 914aeecfe23b1e16af6ea675a4fb5dbd5a5b8d0a Catch2)
endif()

if(NOT EXISTS Argparse.hpp)
  file(DOWNLOAD https://raw.githubusercontent.com/p-ranav/argparse/refs/tags/v3.1/include/argparse/argparse.hpp Argparse.hpp)
endif()