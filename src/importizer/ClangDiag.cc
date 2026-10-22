#include "importizer/ClangDiag.hh"
#include "utils/Diag.hh"
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>
#include <llvm/ADT/SmallString.h>

void MyDiagnosticConsumer::HandleDiagnostic(clang::DiagnosticsEngine::Level lvl,
                                            const clang::Diagnostic &info) noexcept {
  llvm::SmallString<128> msg;
  info.FormatDiagnostic(msg);

  switch (lvl) {
  case clang::DiagnosticsEngine::Level::Ignored:
  case clang::DiagnosticsEngine::Level::Note:
  case clang::DiagnosticsEngine::Level::Remark:
    break;
  case clang::DiagnosticsEngine::Level::Warning:
    warn<false>("{}", msg);
    break;
  case clang::DiagnosticsEngine::Level::Error:
  case clang::DiagnosticsEngine::Level::Fatal:
    err<false>("{}", msg);
    break;
  }
}
