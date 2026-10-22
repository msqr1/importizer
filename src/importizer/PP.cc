#include "importizer/PP.hh"
#include "importizer/Frontend.hh"
#include "importizer/Opts.hh"
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileEntry.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/ModuleLoader.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>

PPAnalyzer::PPAnalyzer(clang::Preprocessor &PP_, FrontendCtx &ctx_)
    : PP{PP_}, ctx{ctx_},
      nestedIncludeDiagID{
          PP.getDiagnostics().getCustomDiagID(
              clang::DiagnosticsEngine::Fatal,
              "#include within conditional block(s) is not modularizable."),
      },
      macroIncludeDiagID{
          PP.getDiagnostics().getCustomDiagID(clang::DiagnosticsEngine::Fatal,
                                              "Macro-expanded #include is not modularizable."),
      } {}

auto PPAnalyzer::FileNotFound(llvm::StringRef /*FileName*/) noexcept -> bool { return false; }
void PPAnalyzer::InclusionDirective(
    clang::SourceLocation hashLoc, const clang::Token & /*IncludeTok*/,
    llvm::StringRef /*FileName*/, bool /*IsAngled*/, clang::CharSourceRange filenameRange,
    clang::OptionalFileEntryRef file, llvm::StringRef /*SearchPath*/,
    llvm::StringRef /*RelativePath*/, const clang::Module * /*SuggestedModule*/,
    bool /*ModuleImported*/, clang::SrcMgr::CharacteristicKind /*FileType*/) noexcept {
  clang::SourceManager &srcMgr{PP.getSourceManager()};

  // Restrict only to the main file, and only resolved #include
  if (!(srcMgr.isInMainFile(hashLoc) && file)) {
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
void PPAnalyzer::moduleImport(clang::SourceLocation /*ImportLoc*/, clang::ModuleIdPath /*Path*/,
                              const clang::Module * /*Imported*/) noexcept {}
void PPAnalyzer::If(clang::SourceLocation /*Loc*/, clang::SourceRange /*ConditionRange*/,
                    ConditionValueKind /*ConditionValue*/) noexcept {
  ++condDepth;
}
void PPAnalyzer::Ifdef(clang::SourceLocation /*Loc*/, const clang::Token & /*MacroNameTok*/,
                       const clang::MacroDefinition & /*MD*/) noexcept {
  ++condDepth;
}
void PPAnalyzer::Ifndef(clang::SourceLocation /*Loc*/, const clang::Token & /*MacroNameTok*/,
                        const clang::MacroDefinition & /*MD*/) noexcept {
  ++condDepth;
}
void PPAnalyzer::Endif(clang::SourceLocation /*Loc*/, clang::SourceLocation /*IfLoc*/) noexcept {
  --condDepth;
}
