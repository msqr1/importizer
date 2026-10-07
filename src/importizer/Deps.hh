#include <clang/Basic/FileEntry.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <llvm/ADT/MapVector.h>
#include <vector>

namespace llvm {
class StringRef;
} // namespace llvm

namespace clang {
class SourceLocation;
class Module;
class CharSourceRange;
class SourceRange;
} // namespace clang

// Adjacency list dependency graph for TUs. A -> B means A depends on B
struct Vertex {
  clang::FileEntryRef file;
};
using DepGraph = llvm::MapVector<Vertex, std::vector<Vertex>>;

// Get necessary info from the preprocessor
struct PPInfo : clang::PPCallbacks {
  unsigned int macroLvl;
  clang::SourceManager &srcMgr;
  PPInfo();
  bool FileNotFound(llvm::StringRef fileName) override;
  void
  InclusionDirective(clang::SourceLocation hashLoc,
                     const clang::Token &includeTok, llvm::StringRef fileName,
                     bool isAngled, clang::CharSourceRange filenameRange,
                     clang::OptionalFileEntryRef file,
                     llvm::StringRef searchPath, llvm::StringRef relativePath,
                     const clang::Module *suggestedModule, bool moduleImported,
                     clang::SrcMgr::CharacteristicKind fileType) override;
  void If(clang::SourceLocation loc, clang::SourceRange condRange,
          ConditionValueKind condVal) override;
  void Ifdef(clang::SourceLocation loc, const clang::Token &macroNameTok,
             const clang::MacroDefinition &md) override;
  void Ifndef(clang::SourceLocation loc, const clang::Token &macroNameTok,
              const clang::MacroDefinition &md) override;
  void Endif(clang::SourceLocation loc, clang::SourceLocation ifLoc) override;
};
