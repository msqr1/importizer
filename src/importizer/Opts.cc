#include "importizer/Opts.hh"
#include "fmt/base.h"
#include "tomlc17.h"
#include "tomlcpp.hpp"
#include "utils/Control.hh"
#include "utils/FileOp.hh"
#include "clang/Tooling/JSONCompilationDatabase.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
namespace tl = toml;

// Get fs::path's from TOML.
void tomlGetPaths(const tl::Result &res, std::string_view multipartKey,
                  std::vector<fs::path> &paths) {
  const std::optional<tl::Datum> datum{res.seek(multipartKey.data())};
  if (!datum) {
    return;
  }
  if (datum->type != TOML_ARRAY) {
    exitWithErr("'{}' must be a String Array\n", multipartKey);
  }
  std::vector<tl::Datum> v{*datum->as_vector()};
  paths.clear();
  for (int i{}; i < v.size(); ++i) {
    if (v[i].type != TOML_STRING) {
      exitWithErr("Element #{} of '{}' is not a String\n", i + 1, multipartKey);
    }
    paths.emplace_back(*v[i].as_str());
  }
}
void getOpts(const int argc, const char **argv, Opts &opts) {
  fs::path config{"importizer.toml"};
  int n_pos_args{};
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
          exitWithErr("{} requires a path\n", arg);
        }
        opts.outDir = argv[i];
        continue;
      } else {
        exitWithErr("Unknown option '{}'\n", arg);
      }
    } else {
      if (++n_pos_args > 1) {
        exitWithErr("Too many positional arguments\n");
      }
      config = arg;
    }
  }

  const tl::Result res{tl::parse_file(portableFOpen(config).get())};
  if (!res.ok()) {
    exitWithErr("(TOML) {}\n", res.errmsg());
  }

  // inDir
  std::optional<tl::Datum> datum{res.seek("inDir")};
  if (datum && datum->type == TOML_STRING) {
    opts.inDir = *datum->as_str();
  } else {
    exitWithErr("'inDir' must be specified and as TOML string\n");
  }

  // Make relative to config file instead of CWD
  fs::path configDir{config.parent_path()};
  if (opts.inDir.is_relative()) {
    opts.inDir = configDir / opts.inDir;
  }

  // outDir
  datum = res.seek("outDir");
  if (datum && !opts.outDir.empty()) {
    warn("outDir from CLI will override config file\n");
  } else if (datum && datum->type == TOML_STRING) {
    opts.outDir = *datum->as_str();

    // Make relative to config file instead of CWD
    if (opts.outDir.is_relative()) {
      opts.outDir = configDir / opts.outDir;
    }
  } else if (opts.outDir.empty()) {
    exitWithErr(
        "'outDir' must be specified on CLI or in config file as a String\n");
  }

  // compilationDb & bootstrap (fileHelper)
  datum = res.seek("compilationDb");
  const std::optional<tl::Datum> bootstrap{res.seek("bootstrap")};
  if (datum && bootstrap) {
    warn("'compilationDb' will take precedence over 'bootstrap'\n");
  } else if (datum) {
    if (datum->type != TOML_STRING) {
      exitWithErr("'compilationDb' must be a String\n");
    }
    std::string msg;
    std::unique_ptr<JSONCompilationDatabase> db{
        JSONCompilationDatabase::loadFromFile(
            *datum->as_str(), msg, JSONCommandLineSyntax::AutoDetect)};
    if (!db) {
      exitWithErr("Unable to parse compilation database: {}\n", msg);
    }
    opts.fileHelper.emplace<std::unique_ptr<JSONCompilationDatabase>>(
        std::move(db));
  } else {
    Bootstrap b{{"h", "hh", "hpp", "hxx", ""},
                {"cc", "cpp", "cxx", "c++", "C"}};
    if (bootstrap) {
      if (bootstrap->type != TOML_TABLE) {
        exitWithErr("'bootstrap' must be a Table\n");
      }
      tomlGetPaths(res, "bootstrap.hdrExts", b.hdrExts);
      tomlGetPaths(res, "bootstrap.srcExts", b.srcExts);
      tomlGetPaths(res, "bootstrap.includePaths", b.includePaths);

      // Make relative to config file instead of CWD
      for (fs::path &path : b.includePaths) {
        if (path.is_relative()) {
          path = configDir / path;
        }
      }
    }
    opts.fileHelper.emplace<Bootstrap>(std::move(b));
  }
}
