#pragma once
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>

struct LogOpts {
  llvm::StringRef prog;
  llvm::raw_ostream *target;
} extern *logOpts;

template <typename... Ts> void err(llvm::StringRef fmt, Ts &&...args) noexcept {
  llvm::WithColor::error(*logOpts->target, logOpts->prog);
  llvm::WithColor{*logOpts->target, llvm::raw_ostream::Colors::SAVEDCOLOR, true}
      << llvm::formatv(fmt.data(), std::forward<Ts>(args)...) << ".\n";
}

template <typename... Ts>
void warn(llvm::StringRef fmt, Ts &&...args) noexcept {
  llvm::WithColor::warning(*logOpts->target, logOpts->prog);
  llvm::WithColor{*logOpts->target, llvm::raw_ostream::Colors::SAVEDCOLOR, true}
      << llvm::formatv(fmt.data(), std::forward<Ts>(args)...) << ".\n";
}
