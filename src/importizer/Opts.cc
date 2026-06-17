#include "importizer/Opts.hh"
#include "importizer/Toml.hh"
#include "utils/Log.hh"
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
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
  cl::OptionCategory cat{logOpts->prog};
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
                                   logOpts->target)) {
    return false;
  }

  const TomlResult res{toml_parse_file_ex(config.c_str())};
  if (!res.ok) {
    err(res.errmsg);
    return false;
  }

  // inDir
  toml_datum_t datum{res.seek("inDir")};
  if (datum.type == TOML_STRING) {
    opts.inDir = datum.u.s;
  } else {
    err("'inDir' must be specified and as String");
    return false;
  }
  llvm::SmallString<128> tmp;
  llvm::StringRef configDir{pth::parent_path(config)};

  // Make relative to config file instead of CWD
  if (pth::is_relative(opts.inDir)) {
    tmp = configDir;
    pth::append(tmp, opts.inDir);
    opts.inDir = std::move(tmp);
  }

  // outDir
  datum = res.seek("outDir");
  if (datum.type && !opts.outDir.empty()) {
    warn("outDir from CLI will override config file");
  } else if (datum.type == TOML_STRING) {
    opts.outDir = datum.u.s;

    // Make relative to config file instead of CWD
    if (pth::is_relative(opts.outDir)) {
      tmp = configDir;
      pth::append(tmp, opts.outDir);
      opts.outDir = std::move(tmp);
    }
  } else if (opts.outDir.empty()) {
    err("'outDir' must be specified on CLI or in config file as a String");
    return false;
  }

  // compilationDb & bootstrap (fileHelper)
  datum = res.seek("compilationDb");
  const toml_datum_t bootstrap{res.seek("bootstrap")};
  if (datum.type && bootstrap.type) {
    warn("'compilationDb' will take precedence over 'bootstrap'");
  } else if (datum.type) {
    if (datum.type != TOML_STRING) {
      err("'compilationDb' must be a String");
      return false;
    }
    std::string msg;
    auto db{tl::JSONCompilationDatabase::loadFromFile(
        datum.u.s, msg, tl::JSONCommandLineSyntax::AutoDetect)};
    if (!db) {
      err("Unable to parse compilation database: {}", msg);
      return false;
    }
    opts.fileHelper.emplace<std::unique_ptr<tl::JSONCompilationDatabase>>(
        std::move(db));
    return true;
  }
  Bootstrap &b{opts.fileHelper.emplace<Bootstrap>()};
  b.ignores.emplace_back("CMakeLists.txt");
  if (!bootstrap.type) {
    return true;
  }
  if (bootstrap.type != TOML_TABLE) {
    err("'bootstrap' must be a Table.");
    return false;
  }
  if (!(res.seekStrs("bootstrap.ignores", b.ignores) &&
        res.seekStrs("bootstrap.includePaths", b.includePaths))) {
    return false;
  }

  // Make relative to config file instead of CWD
  for (llvm::SmallString<128> &p : b.includePaths) {
    if (!pth::is_relative(p)) {
      continue;
    }
    tmp = configDir;
    pth::append(tmp, p);
    p = std::move(tmp);
  }
  return true;
}
