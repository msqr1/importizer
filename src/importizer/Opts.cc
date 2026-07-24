#include "importizer/Opts.hh"
#include "utils/Glob.hh"
#include "utils/Log.hh"
#include "utils/Toml.hh"
#include <array>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <cstddef>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <optional>
#include <string>
#include <tomlc17.h>
#include <utility>

namespace cl = llvm::cl;
namespace tl = clang::tooling;
namespace pth = llvm::sys::path;

template <unsigned len>
struct SmallStrParser : public cl::parser<llvm::SmallString<len>> {
  bool parse(cl::Option &, llvm::StringRef, llvm::StringRef val,
             llvm::SmallString<len> &dst) const {
    dst = val;
    return false;
  }
  SmallStrParser(cl::Option &opt) : cl::parser<llvm::SmallString<len>>{opt} {}
};

bool getOpts(const int argc, const char *const *argv, Opts &opts) noexcept {
  // LLVM default options will mix into ours if we don't make our own category
  cl::OptionCategory cat{g_logOpts->prog};
  cl::opt<llvm::SmallString<128>, false, SmallStrParser<128>> config{
      cl::cat(cat),
      cl::desc("<configuration file>"),
      cl::init(llvm::StringRef{"importizer.toml"}),
      cl::Positional,
      cl::ValueOptional,
  };
  cl::opt<llvm::SmallString<128>, true, SmallStrParser<128>> outDir{
      cl::cat(cat),
      "outDir",
      cl::desc(
          "Override the output directory specified in the configuration file"),
      cl::value_desc("directory"),
      cl::location(opts.outDir),
  };
  cl::alias _{"o", cl::aliasopt(outDir)};
  cl::SetVersionPrinter([](llvm::raw_ostream &s) { s << "3.0.0\n"; });
  cl::HideUnrelatedOptions(cat);
  auto &optMap{cl::getRegisteredOptions()};

  // Reset default descriptions to be consistent with the README
  optMap["help"]->setDescription("Display available options");
  optMap["help-list"]->setDescription("Display list of available options");
  optMap["version"]->setDescription("Display version");

  if (!cl::ParseCommandLineOptions(argc, argv,
                                   "importizer - Automagically rewrite "
                                   "header-based C++ into using modules",
                                   g_logOpts->target)) {
    return false;
  }

  const TomlResult res{toml_parse_file_ex(config.c_str())};
  if (!res.ok) {
    err(res.errmsg);
    return false;
  }

  // inDir
  toml_datum_t datum{toml_get(res, "inDir")};
  if (datum.type != TOML_STRING) {
    err("inDir must be specified and as a string");
    return false;
  }
  opts.inDir = {datum.u.s, static_cast<size_t>(datum.u.str.len)};

  llvm::SmallString<128> tmp;
  llvm::StringRef configDir{pth::parent_path(config)};

  /// Make relative to config file instead of CWD
  if (pth::is_relative(opts.inDir)) {
    tmp = configDir;
    pth::append(tmp, opts.inDir);
    opts.inDir = std::move(tmp);
  }

  // outDir
  datum = toml_get(res, "outDir");
  if (datum.type && !opts.outDir.empty()) {
    warn("outDir from CLI will override config file");
  } else if (opts.outDir.empty()) {
    if (datum.type != TOML_STRING) {
      err("outDir must be specified on CLI or in config file as a string");
      return false;
    }
    opts.outDir = {datum.u.s, static_cast<size_t>(datum.u.str.len)};
  }

  /// Make relative to config file instead of CWD
  if (pth::is_relative(opts.outDir)) {
    tmp = configDir;
    pth::append(tmp, opts.outDir);
    opts.outDir = std::move(tmp);
  }

  datum = toml_get(res, "compilationDb");
  const toml_datum_t bootstrap{toml_get(res, "bootstrap")};

  // compilationDb
  if (datum.type) {
    if (bootstrap.type) {
      warn("compilationDb will take precedence over bootstrap");
    }
    if (datum.type != TOML_STRING) {
      err("compilationDb must be a string");
      return false;
    }
    std::string msg;
    auto db{tl::JSONCompilationDatabase::loadFromFile(
        {datum.u.s, static_cast<size_t>(datum.u.str.len)}, msg,
        tl::JSONCommandLineSyntax::AutoDetect)};
    if (!db) {
      err("Unable to parse compilation database: {}", msg);
      return false;
    }
    opts.fileHelper.emplace<std::unique_ptr<tl::JSONCompilationDatabase>>(
        std::move(db));
    return true;
  }

  // bootstrap
  if (bootstrap.type && bootstrap.type != TOML_TABLE) {
    err("bootstrap must be a table");
    return false;
  }
  Bootstrap &b{opts.fileHelper.emplace<Bootstrap>()};

  // bootstrap.globs
  datum = toml_get(bootstrap, "globs");
  if (datum.type) {
    if (datum.type != TOML_ARRAY) {
      err("bootstrap.globs must be an array");
      return false;
    }
    size_t len{static_cast<size_t>(datum.u.arr.size)};
    for (size_t i{}; i < len; ++i) {
      toml_datum_t &elem{datum.u.arr.elem[i]};
      if (elem.type != TOML_STRING) {
        err("Element {} of bootstrap.globs isn't a string", i);
        return false;
      }
      std::optional<Glob> g{
          mkGlob({elem.u.s, static_cast<size_t>(elem.u.str.len)})};
      if (!g) {
        return false;
      }
      b.globs.emplace_back(std::move(*g));
    }
  } else {
    std::array<llvm::StringRef, 1> defaultGlobs{"!CMakeLists.txt"};
    for (llvm::StringRef s : defaultGlobs) {
      std::optional<Glob> g{mkGlob(s)};
      if (!g) {
        return false;
      }
      b.globs.emplace_back(std::move(*g));
    }
  }

  // bootstrap.includePaths
  datum = toml_get(bootstrap, "includePaths");
  if (datum.type) {
    if (datum.type != TOML_ARRAY) {
      err("bootstrap.includePaths must be an array");
      return false;
    }
    size_t len{static_cast<size_t>(datum.u.arr.size)};
    llvm::SmallString<128> path;
    for (size_t i{}; i < len; ++i) {
      toml_datum_t &elem{datum.u.arr.elem[i]};
      if (elem.type != TOML_STRING) {
        err("Element {} of bootstrap.includePaths isn't a string", i);
        return false;
      }
      path = {elem.u.s, static_cast<size_t>(elem.u.str.len)};

      /// Make relative to config file instead of CWD
      if (pth::is_relative(path)) {
        tmp = configDir;
        pth::append(tmp, path);
        path = std::move(tmp);
      }

      b.includePaths.emplace_back(path);
    }
  }
  return true;
}
