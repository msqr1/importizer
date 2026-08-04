set(arch $ENV{IMPORTIZER_ARCH})
set(BUILD_SHARED_LIBS OFF)

set(compileFlags
  -fno-delete-null-pointer-checks
  -fno-strict-aliasing
)
set(linkFlags)

if(arch STREQUAL "amd64")
  list(APPEND compileFlags -fcf-protection=full)
elseif(arch STREQUAL "arm64")
  list(APPEND compileFlags -mbranch-protection=standard)
endif()

if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  list(APPEND compileFlags
    -fvisibility=hidden
    -fstrict-flex-arrays=3
    -fstack-protector-all
  )
  if(LINUX)
    list(APPEND compileFlags -fstack-clash-protection)
    list(APPEND linkFlags
      "-Wl,-z,nodlopen,-z,noexecstack,-z,relro,-z,now,--as-needed,--no-copy-dt-needed-entries"
      "$<$<CONFIG:Release>:-Wl,-s>"
    )
  elseif(APPLE)
    list(APPEND linkFlags
      "$<$<CONFIG:Release>:-Wl,-S,-x,-dead_strip>"
    )
  endif()
elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  list(APPEND compileFlags
    /sdl
    /guard:cf
  )
  list(APPEND linkFlags
    /CETCOMPAT
    /GUARD:CF
  )
  set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
endif()

add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:${compileFlags}>")
add_link_options(${linkFlags})
