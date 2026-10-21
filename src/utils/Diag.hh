// Diagnostics utility functions

#pragma once
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"

struct LogOpts {
  llvm::StringRef prog;
  llvm::raw_ostream *stream;
} extern *g_logOpts;

// Log an error to the global output stream, always return false
template <bool period = true, typename... Ts>
bool err(llvm::StringRef fmt, Ts &&...args) noexcept {
  llvm::WithColor::error(*g_logOpts->stream, g_logOpts->prog);
  llvm::WithColor stream{*g_logOpts->stream,
                         llvm::raw_ostream::Colors::SAVEDCOLOR, true};
  stream << llvm::formatv(fmt.data(), std::forward<Ts>(args)...);
  if constexpr (period) {
    stream << ".\n";
  } else {
    stream << '\n';
  }

  // So we can do return err(...); instead of err(...); return false;
  return false;
}

// Log a warning to the global output stream
template <bool period = true, typename... Ts>
void warn(llvm::StringRef fmt, Ts &&...args) noexcept {
  llvm::WithColor::warning(*g_logOpts->stream, g_logOpts->prog);
  llvm::WithColor stream{*g_logOpts->stream,
                         llvm::raw_ostream::Colors::SAVEDCOLOR, true};
  stream << llvm::formatv(fmt.data(), std::forward<Ts>(args)...);
  if constexpr (period) {
    stream << ".\n";
  } else {
    stream << '\n';
  }
}
