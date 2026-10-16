/// Links the AST and preprocessor information extractor together. A ClangTool
/// will create an instance of this class per file.

#pragma once
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include <memory>

struct Opts;
namespace llvm {
class StringRef;
} // namespace llvm

namespace clang {
class ASTConsumer;
}

namespace tl = clang::tooling;

// Used to pass information in and out of all layers of a ClangTool.
struct FrontendCtx {
  // Inputs (const refs)
  const Opts &opts;

  // Outputs
};

class FrontendAnalyzer : public clang::ASTFrontendAction {
  FrontendCtx &ctx;

public:
  FrontendAnalyzer(FrontendCtx &ctx);
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &CI,
                    llvm::StringRef file) noexcept override;
};

class FrontendAnalyzerFactory : public tl::FrontendActionFactory {
  FrontendCtx &ctx;

public:
  FrontendAnalyzerFactory(FrontendCtx &ctx);
  std::unique_ptr<clang::FrontendAction> create() noexcept override;
};
