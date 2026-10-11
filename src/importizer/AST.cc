#include "importizer/AST.hh"
#include "importizer/Frontend.hh"
#include <llvm/ADT/StringRef.h>

ASTAnalyzer::ASTAnalyzer(llvm::StringRef file, FrontendInfo &info)
    : file{file}, info{info} {}
