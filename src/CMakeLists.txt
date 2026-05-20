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
  set(CPACK_GENERATOR "ZIP")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Static except for glibc, LLVM
  target_link_options(importizer PRIVATE
    -static-libgcc
    -static-libstdc++ # This can makes libc++ static, too.
  )
  set(CPACK_GENERATOR "TGZ")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # MacOS linking is handled by CMAKE_OSX_DEPLOYMENT_TARGET
  set(CPACK_GENERATOR "TGZ")
endif()

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

# For the developer to be able to keep staying in the build directory
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_FILE:importizer>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")

install(
  TARGETS importizer
  DESTINATION .
  RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
)
install(
  RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
  DESTINATION .
  PRE_EXCLUDE_REGEXES
    # Linux system libraries
    "libm\\.so"
    "libc\\.so"
    "libdl\\.so"
    "libpthread\\.so"
    "librt\\.so"
    "libresolv\\.so"
    "libutil\\.so"
    "ld-linux.*\\.so"
    "libgcc_s\\.so"
    "libstdc\\+\\+\\.so"
    # macOS system libraries
    "libSystem\\.B\\.dylib"
    "libc\\+\\+\\.1\\.dylib"
    "libobjc\\.dylib"
    "/usr/lib/swift"
)

set(CPACK_INSTALL_STRIP TRUE)
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
include(CPack)
