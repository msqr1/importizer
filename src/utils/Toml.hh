#pragma once
#include <llvm/ADT/StringRef.h>
#include <tomlc17.h>

// Exceptionless, RAII & more convenient toml_result_t
struct TomlResult : toml_result_t {
  TomlResult(const toml_result_t &result) noexcept;

  // Disallow copying
  TomlResult(const TomlResult &) = delete;

  // Allow moving
  TomlResult(TomlResult &&other) noexcept;
  TomlResult &operator=(TomlResult &&other) noexcept;

  operator const toml_datum_t &() const noexcept;

  ~TomlResult() noexcept;
};
