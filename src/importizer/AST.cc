#include "importizer/AST.hh"
#include "importizer/Frontend.hh"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <clang/AST/ASTContext.h>

namespace {

class SymbolVisitor : public clang::RecursiveASTVisitor<SymbolVisitor> {
  clang::ASTContext &AST;
  FrontendCtx &ctx;

public:
  SymbolVisitor(clang ::ASTContext &AST, FrontendCtx &ctx)
      : AST{AST}, ctx{ctx} {}
};

} // namespace

ASTAnalyzer::ASTAnalyzer(FrontendCtx &ctx) : ctx{ctx} {}
void ASTAnalyzer::HandleTranslationUnit(clang::ASTContext &AST) noexcept {
  SymbolVisitor{AST, ctx}.TraverseDecl(AST.getTranslationUnitDecl());
}
