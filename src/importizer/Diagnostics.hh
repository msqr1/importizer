// Our DiagnosticConsumer that logs to the global log stream instead of the
// default stderr.

#pragma once
#include <clang/Basic/Diagnostic.h>

class MyDiagnosticConsumer : public clang::DiagnosticConsumer {
public:
  void HandleDiagnostic(clang::DiagnosticsEngine::Level lvl,
                        const clang::Diagnostic &info) noexcept override;
};
