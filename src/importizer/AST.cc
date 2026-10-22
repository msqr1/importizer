#include "importizer/AST.hh"
#include "importizer/Frontend.hh"
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>

namespace {

class SymbolVisitor : public clang::RecursiveASTVisitor<SymbolVisitor> {
  clang::ASTContext &AST;
  FrontendCtx &ctx;

public:
  SymbolVisitor(clang::ASTContext &AST_, FrontendCtx &ctx_) : AST{AST_}, ctx{ctx_} {}
};

} // namespace

ASTAnalyzer::ASTAnalyzer(FrontendCtx &ctx_) : ctx{ctx_} {}
void ASTAnalyzer::HandleTranslationUnit(clang::ASTContext &AST) noexcept {
  SymbolVisitor{AST, ctx}.TraverseDecl(AST.getTranslationUnitDecl());
}
