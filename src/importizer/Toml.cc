#include "importizer/Toml.hh"
#include "tomlc17.h"
#include <string_view>

TomlResult::TomlResult(const toml_result_t &result) noexcept
    : toml_result_t{result} {}
TomlResult::~TomlResult() noexcept { toml_free(*this); }

TomlResult::TomlResult(TomlResult &&other) noexcept : toml_result_t{} {
  std::swap(*this, other);
}
TomlResult &TomlResult::operator=(TomlResult &&other) noexcept {
  std::swap(*this, other);
  return *this;
}
toml_datum_t TomlResult::seek(std::string_view multipartKey) const noexcept {
  return toml_seek(toptab, multipartKey.data());
}
