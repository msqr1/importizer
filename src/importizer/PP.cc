#include "importizer/PP.hh"
#include "importizer/Frontend.hh"
#include <clang/Basic/FileEntry.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/PPCallbacks.h>

PPAnalyzer::PPAnalyzer(clang::SourceManager &SM, FrontendInfo &info)
    : macroLvl{0}, SM{SM}, info{info} {}

bool PPAnalyzer::FileNotFound(llvm::StringRef) noexcept { return false; }
void PPAnalyzer::InclusionDirective(
    clang::SourceLocation hashLoc, const clang::Token &includeTok,
    llvm::StringRef fileName, bool angle, clang::CharSourceRange filenameRange,
    clang::OptionalFileEntryRef file, llvm::StringRef searchPath,
    llvm::StringRef relativePath, const clang::Module *suggestedModule,
    bool moduleImported, clang::SrcMgr::CharacteristicKind fileType) noexcept {}

void PPAnalyzer::If(clang::SourceLocation, clang::SourceRange,
                    ConditionValueKind) noexcept {
  ++macroLvl;
}
void PPAnalyzer::Ifdef(clang::SourceLocation, const clang::Token &,
                       const clang::MacroDefinition &) noexcept {
  ++macroLvl;
}
void PPAnalyzer::Ifndef(clang::SourceLocation, const clang::Token &,
                        const clang::MacroDefinition &) noexcept {
  ++macroLvl;
}
void PPAnalyzer::Endif(clang::SourceLocation, clang::SourceLocation) noexcept {
  --macroLvl;
}
