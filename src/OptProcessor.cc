#include "OptProcessor.hpp"
#include "Base.hpp"
#include "../3rdParty/Argparse.hpp"
#include <algorithm>
#include <fmt/std.h>
#include <toml++/toml.hpp>
#include <utility>
#include <vector>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace fs = std::filesystem;
namespace ap = argparse;

namespace {

template <typename keyTp>
auto getTypeCk(const toml::table& tbl, std::string_view key) {
  const toml::node_view<const toml::node> node{tbl[key]};

  // TOML need std::string, not const char*, string_view, etc.
  if constexpr(std::is_convertible_v<keyTp, std::string>) {
    if(node.is_string()) return node.ref<std::string>();
  }
  else if(node.is<keyTp>()) return node.ref<keyTp>();

// Clang++ < 20 can't handle [[noreturn]] in constructors (eg. exitWithErr).
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic push
  exitWithErr("Incorrect TOML type for {}", key);
#pragma GCC diagnostic pop
}
template<typename T> 
auto getOrDefault(const ap::ArgumentParser& parser, std::string_view cliKey,
  const std::optional<toml::table>& tbl, std::string_view configKey, T&& defaultVal) {
  if(parser.is_used(cliKey)) {
    if constexpr(std::is_convertible_v<T, std::string>) {
      return parser.get<std::string>(cliKey);
    }
    else parser.get<T>(cliKey);
  }
  else if(tbl && tbl->contains(configKey)) return getTypeCk<T>(*tbl, configKey);
  if constexpr(std::is_convertible_v<T, std::string>) return std::string{defaultVal};
  else return std::forward<T>(defaultVal);
}

template<typename T> 
auto getMustHave(const toml::table& tbl, std::string_view key) {
  if(!tbl.contains(key)) exitWithErr("{} must be specified", key);
  return getTypeCk<T>(tbl, key);
}

} // namespace

Opts getOptsOrExit(int argc, const char* const* argv) {
  Opts opts;
  ap::ArgumentParser defaultParser("importizer", "0.0.1");
  defaultParser.add_description("C++ include to import converter. Takes you on the way of" 
    " modularization!");
  defaultParser.add_argument("-c", "--config")
    .help("Path to a TOML configuration file")
    .default_value("importizer.toml");
  defaultParser.add_argument("--std-include-to-import")
    .help("Convert standard includes to import std or import std.compat")
    .implicit_value(true);
  defaultParser.add_argument("-l", "--log-current-file")
    .help("Print the current file being processed")
    .implicit_value(true);
  defaultParser.add_argument("--include-guard-pat")
    .help("Regex to match include guards. #pragma once is processed by default");
  defaultParser.add_argument("-i", "--inDir")
    .help("Input directory");
  defaultParser.add_argument("-o", "--outDir")
    .help("Output directory");
  defaultParser.add_argument("--hdr-ext")
    .help("Header file extension");
  defaultParser.add_argument("--src-ext")
    .help("Source (also module implementation unit) file extension");
  defaultParser.add_argument("--module-interface-ext")
    .help("Module interface unit file extension");
  defaultParser.add_argument("--include-paths")
    .help("Include paths searched when converting include to import")
    .nargs(ap::nargs_pattern::at_least_one);
  defaultParser.add_argument("--ignored-hdrs")
    .help("Paths relative to ```inDir``` of header files to ignore. Their paired sources,"
      " if available, will be treated as if they have a ```main()```")
    .nargs(ap::nargs_pattern::at_least_one);
  ap::ArgumentParser transitionalParser{"transitional", "", ap::default_arguments::help};
  transitionalParser.add_argument("--back-compat-hdrs")
    .help("Generate headers that include the module file to preserve #include for users." 
      " Note that in the codebase itself the module file is still included directly.")
    .implicit_value(true);
  transitionalParser.add_argument("--mi-control")
    .help("Header-module switching macro identifier");
  transitionalParser.add_argument("--mi-export-keyword")
    .help("Exported symbol macro identifier");
  transitionalParser.add_argument("--mi-export-block-begin")
    .help("Export block begin macro identifier");
  transitionalParser.add_argument("--mi-export-block-end")
    .help("Export block end macro identifier");
  transitionalParser.add_argument("--export-macros-path")
    .help("Export macros file path relative to outDir");
  defaultParser.add_subparser(transitionalParser);
  defaultParser.parse_args(argc, argv);
  const fs::path configPath{defaultParser.get("-c")};
  const toml::parse_result parseRes{toml::parse_file(configPath.native())};
  if(!parseRes) {
    const toml::parse_error err{parseRes.error()};
    const toml::source_region errSrc{err.source()};
    exitWithErr("TOML++ error: {} at {}({}:{})", err.description(), configPath,
    errSrc.begin.line, errSrc.begin.column);
  }
  const toml::table config{std::move(parseRes.table())};
  const fs::path configDir{configPath.parent_path()};
  opts.stdIncludeToImport = getOrDefault(defaultParser, "--std-include-to-import",
    config, "stdIncludeToImport", false);
  log("stdIncludeToImport: {}", opts.stdIncludeToImport);
  opts.logCurrentFile = getOrDefault(defaultParser, "--log-current-file", config,
    "logCurrentFile", false);
  log("logCurrentFile: {}", false);
  std::optional<fs::path> path{defaultParser.present("-i")};
  opts.inDir = path ?
    std::move(*path) : configDir / getMustHave<std::string>(config, "inDir");
  path = defaultParser.present("-o");
  opts.outDir = path ?
    std::move(*path) : configDir / getMustHave<std::string>(config, "outDir");
  opts.includeGuardPat.reset(getOrDefault(defaultParser, "--include-guard-pat",
    config, "includeGuardPat", R"([^\s]+_H)"));
  opts.hdrExt = getOrDefault(defaultParser, "--hdr-ext", config, "hdrExt", ".hpp");
  log("hdrExt: {}", opts.hdrExt);
  opts.srcExt = getOrDefault(defaultParser, "--src-ext", config, "srcExt", ".cpp");
  log("srcExt: {}", opts.srcExt);
  opts.moduleInterfaceExt = getOrDefault(defaultParser, "--module-interface-ext", config,
    "moduleInterfaceExt", ".cppm");
  auto getPathArr = [&](std::string_view key,
    std::vector<fs::path>& container, const std::optional<fs::path>& prefix) -> void {
    if(!config.contains(key)) return;
    const toml::array arr{*config[key].as_array()};
    fs::path p;
    for(const toml::node& elem : arr) {
      if(!elem.is<std::string>()) exitWithErr("{} must only contains strings", key);
      container.emplace_back(prefix ? *prefix / elem.ref<std::string>() :
        fs::path(elem.ref<std::string>()));
    }
  };
  auto strRval = [](std::string& s){ return std::move(s); };
  std::optional<std::vector<std::string>> pathArr{
    defaultParser.present<std::vector<std::string>>("--include-paths")};
  if(pathArr) {
    opts.includePaths.resize(pathArr->size());
    std::transform(pathArr->begin(), pathArr->end(), opts.includePaths.begin(), strRval);
  }
  else getPathArr("includePaths", opts.includePaths, configDir);
  pathArr = defaultParser.present<std::vector<std::string>>("--ignored-hdrs");
  if(pathArr) {
    opts.ignoredHdrs.resize(pathArr->size());
    std::transform(pathArr->begin(), pathArr->end(), opts.ignoredHdrs.begin(), strRval);
  }
  else getPathArr("ignoredHdrs", opts.ignoredHdrs, std::nullopt);
  opts.transitionalOpts.emplace();
  if(!(config.contains("Transtional") ||
    defaultParser.is_subcommand_used("transitional"))) {
    return opts;
  }
  std::optional<toml::table> transitionalConfig;
  if(config.contains("Transtional")) {
    transitionalConfig = getTypeCk<toml::table>(config, "Transitional");
  }
  else transitionalConfig = std::nullopt;
  opts.transitionalOpts->backCompatHdrs =
    getOrDefault(transitionalParser, "--back-compat-hdrs", transitionalConfig,
    "backCompatHdrs", false);
  opts.transitionalOpts->mi_control =
    getOrDefault(transitionalParser, "--mi-control", transitionalConfig,
    "mi_control", "CPP_MODULES");
  opts.transitionalOpts->mi_exportKeyword =
    getOrDefault(transitionalParser, "--mi-export-keyword", transitionalConfig,
    "mi_exportKeyword", "EXPORT");
  opts.transitionalOpts->mi_exportBlockBegin = 
    getOrDefault(transitionalParser, "--mi-export-block-begin", transitionalConfig,
    "mi_exportBlockBegin", "BEGIN_EXPORT");
  opts.transitionalOpts->mi_exportBlockEnd = 
    getOrDefault(transitionalParser, "--mi-export-block-end", transitionalConfig,
    "mi_exportBlockEnd", "END_EXPORT");
  opts.transitionalOpts->exportMacrosPath =
    getOrDefault(transitionalParser, "--export-macros-path", transitionalConfig,
    "exportMacrosPath", "Export.hpp");
  return opts;
}
