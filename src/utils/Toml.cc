#include "utils/Toml.hh"
#include <tomlc17.h>
#include <utility>

TomlResult::TomlResult(const toml_result_t &result) noexcept
    : toml_result_t{result} {}

TomlResult::~TomlResult() noexcept { toml_free(*this); }

TomlResult::TomlResult(TomlResult &&other) noexcept {
  operator=(std::move(other));
}

TomlResult &TomlResult::operator=(TomlResult &&other) noexcept {
  std::swap(*this, other);
  return *this;
}

TomlResult::operator const toml_datum_t &() const noexcept { return toptab; }
