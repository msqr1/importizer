set(arch $ENV{IMPORTIZER_ARCH})

set(compileOpts)

if(arch STREQUAL "amd64")
  list(APPEND compileOpts -fcf-protection=full)
elseif(arch STREQUAL "arm64")
  list(APPEND compileOpts -mbranch-protection=standard)
endif()

if(LINUX)
  list(APPEND compileOpts -fstack-clash-protection)
  add_link_options("LINKER:-z,nodlopen,-z,noexecstack,-z,relro,-z,now")
endif()

if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  list(APPEND compileOpts
    -fno-omit-frame-pointer
    -fno-delete-null-pointer-checks
    -fno-strict-aliasing
    -fstrict-flex-arrays=3
    -ftrivial-auto-var-init=zero
    -fvisibility=hidden
    -fstack-protector-all
  )
elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  list(APPEND compileOpts
    /Oy-
    /clang:-fno-delete-null-pointer-checks
    /clang:-fno-strict-aliasing
    /clang:-fstrict-flex-arrays=3
    /clang:-ftrivial-auto-var-init=zero
    /sdl
    /guard:cf
    /MT # CMAKE_MSVC_RUNTIME_LIBRARY didn't work!!
  )
  add_link_options("LINKER:/CETCOMPAT,/GUARD:CF")
endif()

add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:${compileOpts}>")
