#pragma once

#include "utils/Log.hh"
#include <concepts>
#include <string_view>
#include <tomlc17.h>
#include <vector>

template <typename T>
concept ConstructibleFromCharPtr = std::constructible_from<T, const char *>;

// Exceptionless, RAII & more convenient toml_result_t
struct TomlResult : toml_result_t {
  TomlResult(const toml_result_t &result) noexcept;
  ~TomlResult() noexcept;

  // Disallow copying
  TomlResult(const TomlResult &) = delete;

  // Allow moving
  TomlResult(TomlResult &&other) noexcept;
  TomlResult &operator=(TomlResult &&other) noexcept;
  toml_datum_t seek(std::string_view multipartKey) const noexcept;

  // Get string-type's from TOML.
  template <ConstructibleFromCharPtr T>
  bool seekStrs(std::string_view multipartKey,
                std::vector<T> &strs) const noexcept {
    const toml_datum_t datum{this->seek(multipartKey)};
    if (!datum.type) {
      return true;
    }
    if (datum.type != TOML_ARRAY) {
      err("'{}' must be a String Array\n", multipartKey);
      return false;
    }

    strs.clear();
    for (int i{}; i < datum.u.arr.size; ++i) {
      toml_datum_t elem{datum.u.arr.elem[i]};
      if (elem.type != TOML_STRING) {
        err("Element #{} of '{}' is not a String\n", i + 1, multipartKey);
        return false;
      }
      strs.emplace_back(elem.u.s);
    }
    return true;
  }
};
