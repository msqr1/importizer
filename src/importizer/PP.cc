#include "importizer/PP.hh"
#include "importizer/Frontend.hh"
#include "importizer/Opts.hh"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileEntry.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/ModuleLoader.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"

PPAnalyzer::PPAnalyzer(clang::Preprocessor &PP, FrontendCtx &ctx)
    : PP{PP}, ctx{ctx},
      nestedIncludeDiagID{PP.getDiagnostics().getCustomDiagID(
          clang::DiagnosticsEngine::Fatal,
          "#include within conditional block(s) is un-modularizable.")},
      macroIncludeDiagID{PP.getDiagnostics().getCustomDiagID(
          clang::DiagnosticsEngine::Fatal,
          "Macro-expanded #include is un-modularizable.")},
      condDepth{0} {}

bool PPAnalyzer::FileNotFound(llvm::StringRef) noexcept { return false; }
void PPAnalyzer::InclusionDirective(
    clang::SourceLocation hashLoc, const clang::Token &, llvm::StringRef, bool,
    clang::CharSourceRange filenameRange, clang::OptionalFileEntryRef file,
    llvm::StringRef, llvm::StringRef, const clang::Module *, bool,
    clang::SrcMgr::CharacteristicKind) noexcept {
  clang::SourceManager &SM{PP.getSourceManager()};

  // Restrict only to the main file, and only resolved #include
  if (!(SM.isInMainFile(hashLoc) && file)) {
    return;
  }

  // Only enforce top-level #include for modularizable headers
  if (ctx.opts.hdrs.contains(file->getFileEntry().tryGetRealPathName())) {
    // Check if #include is nested within a conditional block
    if (condDepth > 0) {
      PP.getDiagnostics().Report(hashLoc, nestedIncludeDiagID);
      return;
    }

    // Check if #include itself or the filename came from a macro expansion
    if (hashLoc.isMacroID() || filenameRange.getBegin().isMacroID()) {
      PP.getDiagnostics().Report(hashLoc, macroIncludeDiagID);
      return;
    }
  }
}
void PPAnalyzer::moduleImport(clang::SourceLocation, clang::ModuleIdPath,
                              const clang::Module *) noexcept {}
void PPAnalyzer::If(clang::SourceLocation, clang::SourceRange,
                    ConditionValueKind) noexcept {
  ++condDepth;
}
void PPAnalyzer::Ifdef(clang::SourceLocation, const clang::Token &,
                       const clang::MacroDefinition &) noexcept {
  ++condDepth;
}
void PPAnalyzer::Ifndef(clang::SourceLocation, const clang::Token &,
                        const clang::MacroDefinition &) noexcept {
  ++condDepth;
}
void PPAnalyzer::Endif(clang::SourceLocation, clang::SourceLocation) noexcept {
  --condDepth;
}
