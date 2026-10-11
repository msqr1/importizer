/// Extract preprocessor-related information such as inclusion and macros to
/// figure out dependencies of a TU. FrontendAction owns a preprocessor which we
/// must register an instance of this to.

#pragma once
#include <clang/Basic/FileEntry.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/PPCallbacks.h>

namespace llvm {
class StringRef;
} // namespace llvm

namespace clang {
class SourceLocation;
class Module;
class CharSourceRange;
class SourceRange;
} // namespace clang

class PPAnalyzer : public clang::PPCallbacks {
  unsigned int macroLvl;
  clang::SourceManager &srcMgr;

public:
  PPAnalyzer();
  bool FileNotFound(llvm::StringRef fileName) noexcept override;
  void InclusionDirective(
      clang::SourceLocation hashLoc, const clang::Token &includeTok,
      llvm::StringRef fileName, bool isAngled,
      clang::CharSourceRange filenameRange, clang::OptionalFileEntryRef file,
      llvm::StringRef searchPath, llvm::StringRef relativePath,
      const clang::Module *suggestedModule, bool moduleImported,
      clang::SrcMgr::CharacteristicKind fileType) noexcept override;
  void If(clang::SourceLocation loc, clang::SourceRange condRange,
          ConditionValueKind condVal) noexcept override;
  void Ifdef(clang::SourceLocation loc, const clang::Token &macroNameTok,
             const clang::MacroDefinition &md) noexcept override;
  void Ifndef(clang::SourceLocation loc, const clang::Token &macroNameTok,
              const clang::MacroDefinition &md) noexcept override;
  void Endif(clang::SourceLocation loc,
             clang::SourceLocation ifLoc) noexcept override;
};
