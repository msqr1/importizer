#include "importizer/Frontend.hh"
#include "importizer/AST.hh"
#include "importizer/PP.hh"
#include "utils/Log.hh"
#include <clang/AST/ASTConsumer.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm/ADT/SmallString.h>
#include <memory>

void MyDiagnosticConsumer::HandleDiagnostic(
    clang::DiagnosticsEngine::Level lvl,
    const clang::Diagnostic &info) noexcept {
  llvm::SmallString<128> msg;
  info.FormatDiagnostic(msg);
  switch (lvl) {
  case clang::DiagnosticsEngine::Level::Ignored:
  case clang::DiagnosticsEngine::Level::Note:
  case clang::DiagnosticsEngine::Level::Remark:
    break;
  case clang::DiagnosticsEngine::Level::Warning:
    warn<false>("{}", msg);
    break;
  case clang::DiagnosticsEngine::Level::Error:
  case clang::DiagnosticsEngine::Level::Fatal:
    err<false>("{}", msg);
    break;
  }
}

bool FrontendAnalyzer::BeginSourceFileAction(
    clang::CompilerInstance &CI) noexcept {
  CI.createDiagnostics(new MyDiagnosticConsumer());
  CI.getPreprocessor().addPPCallbacks(std::make_unique<PPAnalyzer>());
  return true;
}

std::unique_ptr<clang::ASTConsumer>
FrontendAnalyzer::CreateASTConsumer(clang::CompilerInstance &CI,
                                    llvm::StringRef file) noexcept {
  CI.getFrontendOpts().SkipFunctionBodies = true;
  return std::make_unique<ASTAnalyzer>(file);
}
