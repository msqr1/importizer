#add_executable(importizer Util.cc Regex.cc OptProcessor.cc FileOp.cc Driver.cc
#  Preprocessor.cc Preamble.cc Directive.cc Minimizer.cc)
add_executable(importizer llvm.cc)
target_compile_features(importizer PRIVATE cxx_std_23)

# For actions/upload-artifact
add_custom_command(POST_BUILD TARGET importizer
  COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:importizer>"
  "${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:importizer>")
