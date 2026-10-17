#include "importizer/Frontend.hh"
#include "importizer/AST.hh"
#include "importizer/PP.hh"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include <memory>

FrontendAnalyzer::FrontendAnalyzer(FrontendCtx &ctx_) : ctx{ctx_} {}

std::unique_ptr<clang::ASTConsumer>
FrontendAnalyzer::CreateASTConsumer(clang::CompilerInstance &CI,
                                    llvm::StringRef) noexcept {
  clang::Preprocessor &PP{CI.getPreprocessor()};
  PP.addPPCallbacks(std::make_unique<PPAnalyzer>(PP, ctx));
  return std::make_unique<ASTAnalyzer>(ctx);
}

FrontendAnalyzerFactory::FrontendAnalyzerFactory(FrontendCtx &ctx_)
    : ctx{ctx_} {}

std::unique_ptr<clang::FrontendAction>
FrontendAnalyzerFactory::create() noexcept {
  return std::make_unique<FrontendAnalyzer>(ctx);
}
