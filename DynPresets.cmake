# This "dynamic preset" are hardening flags requiring platform or compiler detection.

set(arch $ENV{IMPORTIZER_ARCH})

add_compile_options(

)

if(arch STREQUAL "amd64")
  add_compile_options(-fcf-protection=full)
elseif(arch STREQUAL "arm64")
  add_compile_options(-mbranch-protection=standard)
endif()

if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  add_compile_options(
    -fno-delete-null-pointer-checks
    -fno-strict-aliasing
    -fstrict-flex-arrays=3
    -ftrivial-auto-var-init=zero
    -fvisibility=hidden
    -fstack-protector-all
  )
elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  add_compile_options(
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

if(LINUX)
  add_compile_options(-fstack-clash-protection)
  add_link_options("LINKER:-z,nodlopen,-z,noexecstack,-z,relro,-z,now")
elseif(APPLE)
  add_link_options("LINKER:-bind_at_load")
endif()
