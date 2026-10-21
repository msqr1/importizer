// Our version of Clang's DiagnosticConsumer that logs to the global log stream
// instead of Clang's default.

#pragma once
#include "clang/Basic/Diagnostic.h"

class MyDiagnosticConsumer : public clang::DiagnosticConsumer {
public:
  void HandleDiagnostic(clang::DiagnosticsEngine::Level lvl,
                        const clang::Diagnostic &info) noexcept override;
};
