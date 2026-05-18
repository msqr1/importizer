#include "Opts.hpp"
#include "Util.hpp"
#include "tomlc17.h"
#include <cstdint>
#include <fmt/base.h>
#include <optional>
#include <string_view>
#include <sys/types.h>
#include <tomlcpp.hpp>

Opts getOpts(const int argc, const char **argv) {
  std::string_view config{"importizer.toml"};
  Opts opts;

  uint8_t n_pos_args{};
  std::string_view arg{};
  for (uint8_t i{1}; i < argc; i++) {
    arg = argv[i];
    if (arg.starts_with('-')) {
      if (arg == "-h" || arg == "--help") {
        fmt::print("Help");
      }
      if (arg == "-v" || arg == "--version") {
        fmt::print("2.0.0");
      } else if (arg == "-o" || arg == "--output") {
        if (++i >= argc) {
          exitWithErr("{} requires a path", arg);
        }
        opts.outDir = argv[i];
        continue;
      } else {
        exitWithErr("Unknown option '{}'", argv[i]);
      }
    } else {
      if (++n_pos_args > 1) {
        exitWithErr("Too many positional arguments");
      }
      config = arg;
    }
  }
  toml::Result res{toml_parse_file_ex(config.data())};
  if (!res.ok()) {
    exitWithErr("(TOML) {}", res.errmsg());
  }
  std::optional<toml::Datum> datum{res.seek("inDir")};
  if (datum.has_value()) {
    opts.inDir = datum->as_str().value();
  } else {
    exitWithErr("(TOML) Key 'inDir' must exist");
  }

  datum = res.seek("outDir");
  if (datum.has_value() && !opts.outDir.empty()) {
    warn("Output directory from CLI will override TOML");
  } else if (datum.has_value()) {
    opts.outDir = datum->as_str().value();
  } else if (opts.outDir.empty()) {
    exitWithErr("Output directory must be specified");
  }

  return opts;
}
