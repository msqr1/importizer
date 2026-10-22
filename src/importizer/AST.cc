#include "importizer/AST.hh"
#include "importizer/Frontend.hh"
#include "importizer/Opts.hh"
#include "utils/Diag.hh"
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/RecursiveASTVisitor.h>

namespace {

class SymbolVisitor : public clang::RecursiveASTVisitor<SymbolVisitor> {
  FrontendCtx &ctx;

public:
  SymbolVisitor(FrontendCtx &ctx_) : ctx{ctx_} {}
  auto TraverseDecl(clang::Decl *decl) -> bool {
    decl->dump(*globalLogOpts.stream);
    return err("{}", ctx.opts.root);
  }
};

} // namespace

ASTAnalyzer::ASTAnalyzer(FrontendCtx &ctx_) : ctx{ctx_} {}
void ASTAnalyzer::HandleTranslationUnit(clang::ASTContext &AST) noexcept {
  SymbolVisitor{ctx}.TraverseAST(AST);
}
