# This "dynamic preset" are hardening flags requiring platform or compiler detection.

set(arch $ENV{IMPORTIZER_ARCH})

add_compile_options(
  -fno-delete-null-pointer-checks
  -fno-strict-aliasing
)

if(arch STREQUAL "amd64")
  add_compile_options(-fcf-protection=full)
elseif(arch STREQUAL "arm64")
  add_compile_options(-mbranch-protection=standard)
endif()

if(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  add_compile_options(
    -fvisibility=hidden
    -fstrict-flex-arrays=3
    -fstack-protector-all
  )
  if(LINUX)
    add_compile_options( -fstack-clash-protection)
    add_link_options(
      "-Wl,-z,nodlopen,-z,noexecstack,-z,relro,-z,now,--as-needed,--no-copy-dt-needed-entries"
      "$<$<CONFIG:Release>:-Wl,-s>"
    )
  elseif(APPLE)
    add_link_options(
      "$<$<CONFIG:Release>:-Wl,-S,-x,-dead_strip>"
    )
  endif()
elseif(CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  add_compile_options(
    /sdl
    /guard:cf

    # CMAKE_MSVC_RUNTIME_LIBRARY didn't work!!
    /MT
  )
  add_link_options(
    /CETCOMPAT
    /GUARD:CF
  )
endif()
