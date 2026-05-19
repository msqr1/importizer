add_executable(importizer
  Driver.cc
  Opts.cc
  Util.cc
  ${tomlc17_SOURCE_DIR}/src/tomlc17.c
)
target_compile_features(importizer PRIVATE cxx_std_23)

target_compile_options(importizer PRIVATE
  -Wall
  -fno-rtti
  $<$<CONFIG:Debug>:-fno-omit-frame-pointer -fsanitize=undefined>
  $<$<CONFIG:Release>:-flto>
)
target_link_options(importizer PRIVATE
  $<$<CONFIG:Debug>:-fsanitize=undefined>
  $<$<CONFIG:Release>:-flto>
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # MSYS2 Clang64 allows fully static
  target_link_options(importizer PRIVATE -static)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Static except for glibc, LLVM
  target_link_options(importizer PRIVATE
    -static-libgcc
    -static-libstdc++ # This can makes libc++ static, too.
  )
endif()
# MacOS is handled by CMAKE_OSX_DEPLOYMENT_TARGET

target_include_directories(importizer PRIVATE ${tomlc17_SOURCE_DIR}/src)
target_include_directories(importizer PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(importizer PRIVATE
  fmt::fmt
  clangAST
  clangBasic
#  clangDriver
  clangFrontend
#  clangOptions
  clangRewriteFrontend
  clangSerialization
#  clangStaticAnalyzerFrontend
#  clangTooling
#  clangToolingSyntax
)

if(CUSTOM_EXE_SUFFIX)
  set_target_properties(importizer PROPERTIES OUTPUT_NAME "importizer-${CUSTOM_EXE_SUFFIX}")
endif()

# For the developer to be able to stay in the build directory
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:importizer>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")
