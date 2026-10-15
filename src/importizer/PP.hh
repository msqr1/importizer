/// Extract preprocessor-related information such as inclusion and macros to
/// figure out dependencies of a TU. FrontendAction owns a preprocessor which we
/// must register an instance of this to.

#pragma once
#include <clang/Basic/FileEntry.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>

struct FrontendInfo;

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
  clang::Preprocessor &PP;
  FrontendInfo &info;
  const unsigned nestedIncludeDiagID;
  const unsigned macroIncludeDiagID;
  unsigned condDepth;

public:
  PPAnalyzer(clang::Preprocessor &PP, FrontendInfo &info);
  bool FileNotFound(llvm::StringRef) noexcept override;
  void InclusionDirective(
      clang::SourceLocation hashLoc, const clang::Token &includeTok,
      llvm::StringRef fileName, bool angle,
      clang::CharSourceRange filenameRange, clang::OptionalFileEntryRef file,
      llvm::StringRef searchPath, llvm::StringRef relativePath,
      const clang::Module *suggestedModule, bool moduleImported,
      clang::SrcMgr::CharacteristicKind fileType) noexcept override;
  void If(clang::SourceLocation, clang::SourceRange,
          ConditionValueKind) noexcept override;
  void Ifdef(clang::SourceLocation, const clang::Token &,
             const clang::MacroDefinition &) noexcept override;
  void Ifndef(clang::SourceLocation, const clang::Token &,
              const clang::MacroDefinition &) noexcept override;
  void Endif(clang::SourceLocation, clang::SourceLocation) noexcept override;
};
