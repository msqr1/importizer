/// Links the AST and preprocessor information extractor together. A ClangTool
/// will create an instance of this class per file.

#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/FrontendAction.h>
#include <memory>

namespace llvm {
class StringRef;
} // namespace llvm

namespace clang {
class ASTConsumer;
}

struct FrontendInfo {};

class MyDiagnosticConsumer : public clang::DiagnosticConsumer {
public:
  void HandleDiagnostic(clang::DiagnosticsEngine::Level lvl,
                        const clang::Diagnostic &info) noexcept override;
};

class FrontendAnalyzer : public clang::ASTFrontendAction {

public:
  bool BeginSourceFileAction(clang::CompilerInstance &CI) noexcept override;
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI,
                    llvm::StringRef file) noexcept override;
};
