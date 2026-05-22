#include "Opts.hpp"
#include "Util.hpp"
#include "tomlc17.h"
#include <cstdint>
#include <filesystem>
#include <fmt/base.h>
#include <optional>
#include <string_view>
#include <sys/types.h>
#include <tomlcpp.hpp>

namespace fs = std::filesystem;
Opts getOpts(const int argc, const char **argv) {
  Opts opts;
  fs::path config{"importizer.toml"};
  uint8_t n_pos_args{};
  std::string_view arg{};
  for (int i{1}; i < argc; i++) {
    arg = argv[i];
    if (arg.starts_with('-')) {
      if (arg == "-h" || arg == "--help") {
        fmt::println("importizer - native C++20 modularization tool");
        exitOk();
      } else if (arg == "-v" || arg == "--version") {
        fmt::println("3.0.0");
        exitOk();
      } else if (arg == "-o" || arg == "--outDir") {
        if (++i >= argc) {
          exitWithErr("{} requires a path", arg);
        }
        opts.outDir = argv[i];
        continue;
      } else {
        exitWithErr("Unknown option '{}'", arg);
      }
    } else {
      if (++n_pos_args > 1) {
        exitWithErr("Too many positional arguments");
      }
      config = arg;
    }
  }
  toml::Result res{toml_parse_file_ex(config.c_str())};
  if (!res.ok()) {
    exitWithErr("(TOML) {}", res.errmsg());
  }
  std::optional<toml::Datum> datum{res.seek("inDir")};
  if (datum.has_value()) {
    opts.inDir = datum->as_str().value();
  } else {
    exitWithErr("'inDir' must be specified");
  }

  // Make inDir relative to config file instead of CWD
  if (opts.inDir.is_relative()) {
    opts.inDir = config.parent_path() / opts.inDir;
  }
  if (!fs::is_directory(opts.inDir)) {
    exitWithErr("inDir must be an existing directory");
  }

  datum = res.seek("outDir");
  if (datum.has_value() && !opts.outDir.empty()) {
    warn("outDir from CLI will override config file");
  } else if (datum.has_value()) {
    opts.outDir = datum->as_str().value();

    // Make outDir relative to config file instead of CWD
    if (opts.outDir.is_relative()) {
      opts.outDir = config.parent_path() / opts.outDir;
    }
  } else if (opts.outDir.empty()) {
    exitWithErr("'outDir' must be specified (on CLI or in config file)");
  }

  if (fs::is_directory(opts.outDir) && !fs::is_empty(opts.outDir)) {
    exitWithErr("outDir must be empty or not exist");
  }
  return opts;
}
