#include "importizer/AST.hh"
#include "importizer/Config.hh"
#include "importizer/Frontend.hh"
#include "utils/Diag.hh"
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/RecursiveASTVisitor.h>

namespace {

class SymbolVisitor : public clang::RecursiveASTVisitor<SymbolVisitor> {
  FrontendCtx &ctx_;

public:
  SymbolVisitor(FrontendCtx &ctx) : ctx_{ctx} {}
  auto traverseDecl(clang::Decl *decl) -> bool {
    decl->dump(*globalLogCfg.stream());
    logErr("{}", ctx_.cfg.rootDir);
    return false;
  }
};

} // namespace

ASTAnalyzer::ASTAnalyzer(FrontendCtx &ctx) : ctx_{ctx} {}
void ASTAnalyzer::HandleTranslationUnit(clang::ASTContext &AST) noexcept {
  SymbolVisitor{ctx_}.TraverseAST(AST);
}
