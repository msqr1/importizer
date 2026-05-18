add_executable(importizer
  Llvm.cc
  ${tomlc17_SOURCE_DIR}/src/tomlc17.c
)
target_compile_features(importizer PRIVATE cxx_std_23)

target_include_directories(importizer PRIVATE ${tomlc17_SOURCE_DIR}/src)
target_include_directories(importizer PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(importizer PRIVATE
  fmt::fmt
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangSerialization
  clangTooling
)

if(CUSTOM_EXE_SUFFIX)
  set_target_properties(importizer PROPERTIES OUTPUT_NAME "importizer-${CUSTOM_EXE_SUFFIX}")
endif()


# For actions/upload-artifact
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:importizer>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")
