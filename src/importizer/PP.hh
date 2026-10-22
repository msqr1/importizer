// Extract preprocessor-related information such as inclusion and macros to
// figure out dependencies of a TU. FrontendAction owns a preprocessor which we
// must register an instance of this to.

#pragma once
#include <clang/Basic/FileEntry.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/ModuleLoader.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>

struct FrontendCtx;
struct Opts;

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
  FrontendCtx &ctx;
  const unsigned nestedIncludeDiagID;
  const unsigned macroIncludeDiagID;
  unsigned condDepth{0};

public:
  PPAnalyzer(clang::Preprocessor &PP, FrontendCtx &ctx);
  auto FileNotFound(llvm::StringRef /*FileName*/) noexcept -> bool override;
  void InclusionDirective(clang::SourceLocation hashLoc, const clang::Token &includeTok,
                          llvm::StringRef fileName, bool angle,
                          clang::CharSourceRange filenameRange, clang::OptionalFileEntryRef file,
                          llvm::StringRef searchPath, llvm::StringRef relativePath,
                          const clang::Module *suggestedModule, bool moduleImported,
                          clang::SrcMgr::CharacteristicKind fileType) noexcept override;
  void moduleImport(clang::SourceLocation importLoc, clang::ModuleIdPath path,
                    const clang::Module *imported) noexcept override;
  void If(clang::SourceLocation loc, clang::SourceRange condRange,
          ConditionValueKind condVal) noexcept override;
  void Ifdef(clang::SourceLocation cond, const clang::Token &macroNameTok,
             const clang::MacroDefinition &MD) noexcept override;
  void Ifndef(clang::SourceLocation cond, const clang::Token &macroNameTok,
              const clang::MacroDefinition &MD) noexcept override;
  void Endif(clang::SourceLocation loc, clang::SourceLocation ifLoc) noexcept override;
};
