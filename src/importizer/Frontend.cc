#include "importizer/Frontend.hh"
#include "importizer/AST.hh"
#include "importizer/PP.hh"
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm/ADT/SmallString.h>
#include <memory>

std::unique_ptr<clang::ASTConsumer>
FrontendAnalyzer::CreateASTConsumer(clang::CompilerInstance &CI,
                                    llvm::StringRef file) noexcept {
  CI.getPreprocessor().addPPCallbacks(
      std::make_unique<PPAnalyzer>(CI.getSourceManager(), info));
  CI.getFrontendOpts().SkipFunctionBodies = true;
  return std::make_unique<ASTAnalyzer>(file, info);
}

FrontendAnalyzerFactory::FrontendAnalyzerFactory(FrontendInfo &info)
    : info{info} {}

std::unique_ptr<clang::FrontendAction>
FrontendAnalyzerFactory::create() noexcept {
  return std::make_unique<FrontendAnalyzer>(info);
}
