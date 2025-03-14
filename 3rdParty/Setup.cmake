find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "Git not found")
endif()

function(gitClone repo hash outDir)
  file(MAKE_DIRECTORY ${outDir})
  execute_process(WORKING_DIRECTORY ${outDir} COMMAND ${GIT_EXECUTABLE} init)
  execute_process(WORKING_DIRECTORY ${outDir} COMMAND ${GIT_EXECUTABLE} fetch --depth 1
    "https://github.com/${repo}" ${hash})
  execute_process(WORKING_DIRECTORY ${outDir} COMMAND ${GIT_EXECUTABLE} -c advice.detachedHead=false
    checkout ${hash})
  execute_process(WORKING_DIRECTORY ${outDir} COMMAND ${GIT_EXECUTABLE} submodule update --init
   --recursive --depth 1)
endfunction(gitClone repo hash outDir)

# 11.1.4
if(NOT EXISTS fmt)
  gitClone(fmtlib/fmt 123913715afeb8a437e6388b4473fcc4753e1c9a fmt)
endif()

# 3.4.0
if(NOT EXISTS toml++)
  gitClone(marzer/tomlplusplus 30172438cee64926dc41fdd9c11fb3ba5b2ba9de toml++)
endif()

# 10.45
if(NOT EXISTS pcre2)
  gitClone(PCRE2Project/pcre2 2dce7761b1831fd3f82a9c2bd5476259d945da4d pcre2)
endif()
if(NOT EXISTS args.hpp)
  file(DOWNLOAD https://raw.githubusercontent.com/Taywee/args/refs/tags/6.4.6/args.hxx args.hpp)
endif()