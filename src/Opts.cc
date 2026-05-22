#include "Opts.hh"
#include "Util.hh"
#include "fmt/base.h"
#include "tomlc17.h"
#include "tomlcpp.hpp"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace fs = std::filesystem;
namespace tl = toml;

// For handling wchar_t paths on Windows
struct FileCloser {
  void operator()(FILE *fp) const { std::fclose(fp); }
};
using File = std::unique_ptr<FILE, FileCloser>;
File portableFOpen(const fs::path &path) {
  std::FILE *fp;
#ifdef WIN32
  char msg[128];
  errno_t err{_wfopen_s(&fp, path.c_str(), L"r")};
  if (err != 0) {
    strerror_s(msg, 128, err);
    exitWithErr("Unable to open {}, {}", path, msg);
  }
#else
  fp = std::fopen(path.c_str(), "r");
  if (fp == nullptr) {
    exitWithErr("Unable to open {}, {}", path, std::strerror(errno));
  }
#endif
  return File(fp);
}
void getOpts(const int argc, const char **argv, Opts &opts) {
  fs::path configPath{"importizer.toml"};
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
      configPath = arg;
    }
  }
  tl::Result res{tl::parse_file(portableFOpen(configPath).get())};
  if (!res.ok()) {
    exitWithErr("(TOML) {}", res.errmsg());
  }

  // inDir
  std::optional<tl::Datum> datum{res.seek("inDir")};
  if (datum.has_value() && datum->type == TOML_STRING) {
    opts.inDir = datum->as_str().value();
  } else {
    exitWithErr("'inDir' must be specified and as TOML string");
  }

  // Make relative to config file instead of CWD
  if (opts.inDir.is_relative()) {
    opts.inDir = configPath.parent_path() / opts.inDir;
  }
  if (!fs::is_directory(opts.inDir)) {
    exitWithErr("inDir must be an existing directory");
  }

  // outDir
  datum = res.seek("outDir");
  if (datum.has_value() && !opts.outDir.empty()) {
    warn("outDir from CLI will override config file");
  } else if (datum.has_value() && datum->type == TOML_STRING) {
    opts.outDir = datum->as_str().value();

    // Make relative to config file instead of CWD
    if (opts.outDir.is_relative()) {
      opts.outDir = configPath.parent_path() / opts.outDir;
    }
  } else if (opts.outDir.empty()) {
    exitWithErr(
        "'outDir' must be specified on CLI or in config file as a String");
  }

  if (fs::is_directory(opts.outDir) && !fs::is_empty(opts.outDir)) {
    exitWithErr("outDir must be empty or not exist");
  }

  // compilationDb & bootstrap (fileHelper)
  datum = res.seek("compilationDb");
  std::optional<tl::Datum> bootstrap{res.seek("bootstrap")};
  if (datum.has_value() && bootstrap.has_value()) {
    warn("'compilationDb' will take precedence over 'bootstrap'");
  } else if (datum.has_value()) {
    if (datum->type != TOML_STRING) {
      exitWithErr("'compilationDb' should be a String");
    }
    std::string msg;
    std::unique_ptr<JSONCompilationDatabase> db{
        JSONCompilationDatabase::loadFromFile(
            datum->as_str().value(), msg, JSONCommandLineSyntax::AutoDetect)};
    if (db == nullptr) {
      exitWithErr("Unable to parse compilation database: {}", msg);
    }
    opts.fileHelper.emplace<std::unique_ptr<JSONCompilationDatabase>>(
        std::move(db));
  } else {
    Bootstrap b;
    if (bootstrap.has_value()) {

    } else {
      b.hdrExts = {"h", "hh", "hpp", "hxx", ""};
      b.srcExts = {"cc", "cpp", "cxx", "c++", "C"};
    }
    opts.fileHelper.emplace<Bootstrap>(std::move(b));
  }
}
