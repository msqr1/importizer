#include "importizer/Preproc.hh"
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <llvm/ADT/StringRef.h>

const clang::PreprocessorOptions opts{};

bool preproc(llvm::StringRef file) noexcept {}
