// Diagnostics utility functions

#pragma once
#include <cassert>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>

struct LogOpts {
  mutable const char *prog;
  mutable llvm::raw_ostream *stream;
} const extern globalLogOpts;

void setGlobalLogOpts(const LogOpts &opts);

// Log an error to the global output stream, always return false
template <bool period = true, typename... Ts>
auto err(const char *fmt, Ts &&...args) noexcept -> bool {
  assert(globalLogOpts.stream != nullptr);
  llvm::WithColor::error(*globalLogOpts.stream, globalLogOpts.prog);
  llvm::WithColor stream{*globalLogOpts.stream,
                         llvm::raw_ostream::Colors::SAVEDCOLOR, true};
  stream << llvm::formatv(fmt, std::forward<Ts>(args)...);
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
void warn(const char *fmt, Ts &&...args) noexcept {
  assert(globalLogOpts.stream != nullptr);
  llvm::WithColor::warning(*globalLogOpts.stream, globalLogOpts.prog);
  llvm::WithColor stream{*globalLogOpts.stream,
                         llvm::raw_ostream::Colors::SAVEDCOLOR, true};
  stream << llvm::formatv(fmt, std::forward<Ts>(args)...);
  if constexpr (period) {
    stream << ".\n";
  } else {
    stream << '\n';
  }
}
