/// Extract AST-related information such as top-level class, functions and
/// variables of a TU to rewrite later. FrontendAction own an instance of this
/// class.

#pragma once
#include "importizer/Frontend.hh"
#include <clang/AST/ASTConsumer.h>
#include <llvm/ADT/StringRef.h>

struct FrontendInfo;

class ASTAnalyzer : public clang::ASTConsumer {
  llvm::StringRef file;
  FrontendInfo &info;

public:
  ASTAnalyzer(llvm::StringRef file, FrontendInfo &info);
};
