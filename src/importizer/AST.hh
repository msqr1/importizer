// Extract AST-related information such as top-level class, functions and
// variables of a TU to rewrite later. FrontendAction own an instance of this
// class.

#pragma once
#include <clang/AST/ASTConsumer.h>

struct FrontendCtx;

class ASTAnalyzer : public clang::ASTConsumer {
  FrontendCtx &ctx;

public:
  ASTAnalyzer(FrontendCtx &ctx);
  void HandleTranslationUnit(clang::ASTContext &AST) noexcept override;
};
