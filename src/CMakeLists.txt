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
  $<$<CONFIG:Debug>:-O0 -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address>
  $<$<CONFIG:Release>:-flto>
)
target_link_options(importizer PRIVATE
  $<$<CONFIG:Debug>:-O0 -fsanitize=undefined -fsanitize=address>
  $<$<CONFIG:Release>:-flto>
)
target_include_directories(importizer PRIVATE ${tomlc17_SOURCE_DIR}/src)
target_include_directories(importizer PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(importizer PRIVATE
  fmt::fmt
  clangAST
  clangBasic
  clangTooling
  clangFrontend
  clangRewriteFrontend
  clangSerialization
)
install(
  TARGETS importizer
  DESTINATION .
  RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
)
if(NOT CPACK_PACKAGE_FILE_NAME)
  set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
endif()

set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# Platform-dependent tweaks
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Static except for LLVM and its deps
  target_link_options(importizer PRIVATE
    -static
  )
  set(CPACK_GENERATOR "ZIP")

  # Exclude Windows system libraries
  install(
      RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
      DESTINATION .
      PRE_EXCLUDE_REGEXES
        "api-ms-.*"
        "ext-ms-.*"
      POST_EXCLUDE_REGEXES
        ".*system32.*"
        ".*Windows.*"
      DIRECTORIES ${DLL_DIR}
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Static except for glibc, LLVM and its deps
  target_link_options(importizer PRIVATE
    -static-libgcc
    -static-libstdc++ # This can makes libc++ static, too.
  )
  set(CPACK_GENERATOR "TGZ")

  # Exclude Linux system libraries
  install(
    RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
    DESTINATION .
    PRE_EXCLUDE_REGEXES
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
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # MacOS linking is handled by CMAKE_OSX_DEPLOYMENT_TARGET
  set(CPACK_GENERATOR "TGZ")

  # Exclude MacOS system libraries
  install(
    RUNTIME_DEPENDENCY_SET IMPORTIZER_DEPS
    DESTINATION .
    PRE_EXCLUDE_REGEXES
      "libSystem\\.B\\.dylib"
      "libc\\+\\+\\.1\\.dylib"
      "libobjc\\.dylib"
      "/usr/lib/swift"
    )
endif()
include(CPack)

# For the developer to be able to keep staying in the build directory
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_FILE:importizer>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")
