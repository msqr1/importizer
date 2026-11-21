#include "importizer/Frontend.hh"
#include "importizer/AST.hh"
#include "importizer/PP.hh"
#include <clang/Frontend/FrontendAction.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm/ADT/StringRef.h>
#include <memory>

FrontendAnalyzer::FrontendAnalyzer(FrontendCtx &ctx) : ctx_{ctx} {}

auto FrontendAnalyzer::CreateASTConsumer(clang::CompilerInstance &compiler,
                                         llvm::StringRef /*InFile*/) noexcept
    -> std::unique_ptr<clang::ASTConsumer> {
  clang::Preprocessor &PP{compiler.getPreprocessor()};
  PP.addPPCallbacks(std::make_unique<PPAnalyzer>(PP, ctx_));
  return std::make_unique<ASTAnalyzer>(ctx_);
}

FrontendAnalyzerFactory::FrontendAnalyzerFactory(FrontendCtx &ctx) : ctx_{ctx} {}

auto FrontendAnalyzerFactory::create() noexcept -> std::unique_ptr<clang::FrontendAction> {
  return std::make_unique<FrontendAnalyzer>(ctx_);
}
