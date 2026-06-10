#include "importizer/Opts.hh"
#include "importizer/Toml.hh"
#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <filesystem>
#include <fmt/base.h>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tomlc17.h>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
std::optional<bool> getOpts(const int argc, const char **argv,
                            Opts &opts) noexcept {
  fs::path config{"importizer.toml"};
  int n_pos_args{};
  std::string_view arg;
  for (int i{1}; i < argc; i++) {
    arg = argv[i];

    // Already handled
    if (arg == "-r" || arg == "--raw") {
      ++i;
      continue;
    }

    if (!arg.starts_with('-')) {
      if (++n_pos_args > 1) {
        err("Too many positional arguments.");
        return false;
      }
      config = arg;
      continue;
    }
    if (arg == "-h" || arg == "--help") {
      fmt::println("importizer - native C++20 modularization tool.");
      return std::nullopt;
    }
    if (arg == "-v" || arg == "--version") {
      fmt::println("3.0.0");
      return std::nullopt;
    }
    if (arg == "-o" || arg == "--outDir") {
      if (++i >= argc) {
        err("{} requires a path.", arg);
        return false;
      }
      opts.outDir = argv[i];
      continue;
    }
    err("Unknown option '{}'.", arg);
    return false;
  }

  File f{portableFOpen(config)};
  if (!f) {
    return false;
  }
  const TomlResult res{toml_parse_file(f.get())};
  if (!res.ok) {
    err("Unable to parse config file {}: {}.", config, res.errmsg);
    return false;
  }

  // inDir
  toml_datum_t datum{res.seek("inDir")};
  if (datum.type == TOML_STRING) {
    opts.inDir = datum.u.s;
  } else {
    err("'inDir' must be specified and as String.");
    return false;
  }

  // Make relative to config file instead of CWD
  fs::path configDir{config.parent_path()};
  if (opts.inDir.is_relative()) {
    opts.inDir = configDir / opts.inDir;
  }

  // outDir
  datum = res.seek("outDir");
  if (datum.type && !opts.outDir.empty()) {
    warn("outDir from CLI will override config file.");
  } else if (datum.type == TOML_STRING) {
    opts.outDir = datum.u.s;

    // Make relative to config file instead of CWD
    if (opts.outDir.is_relative()) {
      opts.outDir = configDir / opts.outDir;
    }
  } else if (opts.outDir.empty()) {
    err("'outDir' must be specified on CLI or in config file as a String.");
    return false;
  }

  // compilationDb & bootstrap (fileHelper)
  datum = res.seek("compilationDb");
  const toml_datum_t bootstrap{res.seek("bootstrap")};
  if (datum.type && bootstrap.type) {
    warn("'compilationDb' will take precedence over 'bootstrap'.");
  } else if (datum.type) {
    if (datum.type != TOML_STRING) {
      err("'compilationDb' must be a String.");
      return false;
    }
    std::string msg;
    std::unique_ptr<JSONCompilationDatabase> db{
        JSONCompilationDatabase::loadFromFile(
            datum.u.s, msg, JSONCommandLineSyntax::AutoDetect)};
    if (!db) {
      err("Unable to parse compilation database: {}.", msg);
      return false;
    }
    opts.fileHelper.emplace<std::unique_ptr<JSONCompilationDatabase>>(
        std::move(db));
    return true;
  }
  opts.fileHelper.emplace<Bootstrap>(
      std::vector<fs::path>{"h", "hh", "hpp", "hxx", ""},
      std::vector<fs::path>{"cc", "cpp", "cxx", "c++", "C"});
  Bootstrap &b{std::get<Bootstrap>(opts.fileHelper)};
  if (!bootstrap.type) {
    return true;
  }
  if (bootstrap.type != TOML_TABLE) {
    err("'bootstrap' must be a Table.");
    return false;
  }
  if (!(res.seekStrs("bootstrap.hdrExts", b.hdrExts) &&
        res.seekStrs("bootstrap.srcExts", b.srcExts) &&
        res.seekStrs("bootstrap.includePaths", b.includePaths))) {
    return false;
  }

  // Make relative to config file instead of CWD
  for (fs::path &path : b.includePaths) {
    if (!path.is_relative()) {
      continue;
    }
    path = configDir / path;
  }
  return true;
}
