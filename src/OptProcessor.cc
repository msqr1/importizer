#include "OptProcessor.hpp"
#include "Base.hpp"
#include "../3rdParty/Argparse.hpp"
#include <fmt/format.h>
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
  const std::optional<toml::table>& tbl, std::string_view tblKey, T&& defaultVal) {
  if(parser.is_used(cliKey)) {
    if constexpr(std::is_convertible_v<T, std::string>) {
      return parser.get<std::string>(cliKey);
    }
    else parser.get<T>(cliKey);
  }
  else if(tbl && tbl->contains(tblKey)) return getTypeCk<T>(*tbl, tblKey);
  if constexpr(std::is_convertible_v<T, std::string>) return std::string{defaultVal};
  else return std::forward<T>(defaultVal);
}
template<typename T>
auto getMustHave(const toml::table& tbl, std::string_view key) {
  if(!tbl.contains(key)) exitWithErr("{} must be specified", key);
  return getTypeCk<T>(tbl, key);
}
void getPathArr(const ap::ArgumentParser& parser, std::string_view cliKey,
  const std::optional<toml::table>& tbl, std::string_view tblKey,
  std::vector<fs::path>& container, const std::optional<fs::path>& prefix = std::nullopt) {
  if(std::optional<std::vector<std::string>> pathArr{
    parser.present<std::vector<std::string>>(cliKey)}) {
    for(std::string& str : *pathArr) container.emplace_back(std::move(str));
  }
  else if(tbl->contains(tblKey)) {
    toml::array arr{*tbl->get(tblKey)->as_array()};
    fs::path p;
    for(toml::node& elem : arr) {
      if(!elem.is<std::string>()) exitWithErr("{} must only contains strings", tblKey);
      if(prefix) container.emplace_back(*prefix / std::move(elem.ref<std::string>()));
      else container.emplace_back(std::move(elem.ref<std::string>()));
    }
  }
}

} // namespace
Opts getOptsOrExit(int argc, const char* const* argv) {
  Opts opts;
  ap::ArgumentParser generalParser("importizer", "0.0.1");
  generalParser.add_description("C++ include to import converter. Takes you on the way of"
    " modularization!");
  generalParser.add_argument("-c", "--config")
    .help("Path to a TOML configuration file")
    .default_value("importizer.toml");
  generalParser.add_argument("-s", "--std-include-to-import")
    .help("Convert standard includes to import std or import std.compat")
    .implicit_value(true);
  generalParser.add_argument("-l", "--log-current-file")
    .help("Print the current file being processed")
    .implicit_value(true);
  generalParser.add_argument("--include-guard-pat")
    .help("Regex to match include guards. #pragma once is processed by default");
  generalParser.add_argument("-i", "--in-dir")
    .help("Input directory");
  generalParser.add_argument("-o", "--out-dir")
    .help("Output directory");
  generalParser.add_argument("--hdr-ext")
    .help("Header file extension");
  generalParser.add_argument("--src-ext")
    .help("Source (also module implementation unit) file extension");
  generalParser.add_argument("--module-interface-ext")
    .help("Module interface unit file extension");
  generalParser.add_argument("--include-paths")
    .help("Include paths searched when converting include to import")
    .nargs(ap::nargs_pattern::at_least_one);
  generalParser.add_argument("--ignored-hdrs")
    .help("Paths relative to inDir of headers to ignore. Their paired sources,"
      " if available, will be treated as if they have a main()")
    .nargs(ap::nargs_pattern::at_least_one);
  generalParser.add_argument("--umbrella-hdrs")
    .help("Paths relative to inDir of modularized headers, but their 'import' are turned into 'export import'")
    .nargs(ap::nargs_pattern::at_least_one);
  generalParser.add_argument("--bootstrap-std-module")
    .help("Generate a bootstrap standard modules. Recommended name is something like"
      " 'bootstrapStd'. '.compat' will be added automatically. Ineffective if"
      " stdIncludeToImport is off");
  ap::ArgumentParser transitionalParser{"transitional", "", ap::default_arguments::help};
  transitionalParser.add_argument("-b", "--back-compat-hdrs")
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
  generalParser.add_subparser(transitionalParser);
  generalParser.parse_args(argc, argv);
  const fs::path configPath{generalParser.get("-c")};
  toml::parse_result parseRes{toml::parse_file(configPath.native())};
  if(!parseRes) {
    toml::parse_error err{std::move(parseRes.error())};
    const toml::source_region errSrc{std::move(err.source())};
    exitWithErr("TOML++ error: {} at {}({}:{})", err.description(), configPath,
    errSrc.begin.line, errSrc.begin.column);
  }
  const toml::table config{std::move(parseRes.table())};
  const fs::path configDir{configPath.parent_path()};
  opts.stdIncludeToImport = getOrDefault(generalParser, "-s", config,
    "stdIncludeToImport", false);
  opts.logCurrentFile = getOrDefault(generalParser, "-l", config, "logCurrentFile", false);
  std::optional<fs::path> path{generalParser.present("-i")};
  opts.inDir = path ?
    std::move(*path) : configDir / getMustHave<std::string>(config, "inDir");
  path = generalParser.present("-o");
  opts.outDir = path ?
    std::move(*path) : configDir / getMustHave<std::string>(config, "outDir");
  opts.includeGuardPat.reset(fmt::format(
    "^{}$",
    getOrDefault(generalParser, "--include-guard-pat",
    config, "includeGuardPat", R"([^\s]+_H)")));
  opts.hdrExt = getOrDefault(generalParser, "--hdr-ext", config, "hdrExt", ".hpp");
  opts.srcExt = getOrDefault(generalParser, "--src-ext", config, "srcExt", ".cpp");
  opts.moduleInterfaceExt = getOrDefault(generalParser, "--module-interface-ext", config,
    "moduleInterfaceExt", ".cppm");
  getPathArr(generalParser, "--include-paths", config, "includePaths", opts.includePaths,
    configDir);
  getPathArr(generalParser, "--ignored-hdrs", config, "ignoredHdrs", opts.ignoredHdrs);
  getPathArr(generalParser, "--umbrella-hdrs", config, "umbrellaHdrs", opts.umbrellaHdrs);
  if(opts.stdIncludeToImport && (generalParser.is_used("--bootstrap-std-module")
    || config.contains("bootstrapStdModule"))) {
    getOrDefault(generalParser, "--bootstrap-std-module", config, "bootstrapStdModule",
    "");
  }
  else opts.bootstrapStdModule = std::nullopt;
  /*
  log(
    "stdIncludeToImport: {}\n"
    "logCurrentFile: {}\n"
    "inDir: {}\n"
    "outDir: {}\n"
    "hdrExt: {}\n"
    "srcExt: {}\n"
    "moduleInterfaceExt: {}\n",
    opts.stdIncludeToImport, opts.logCurrentFile, opts.inDir, opts.outDir, opts.hdrExt,
    opts.srcExt, opts.moduleInterfaceExt);
  */
  if(!(config.contains("Transitional") ||
    generalParser.is_subcommand_used("transitional"))) {
    opts.transitionalOpts = std::nullopt;
    return opts;
  }
  TransitionalOpts& tOpts{opts.transitionalOpts.emplace()};
  std::optional<toml::table> tConfig;
  if(config.contains("Transitional")) {
    tConfig = getTypeCk<toml::table>(config, "Transitional");
  }
  else tConfig = std::nullopt;
  tOpts.backCompatHdrs = getOrDefault(transitionalParser, "-b", tConfig,
    "backCompatHdrs", false);
  tOpts.mi_control = getOrDefault(transitionalParser, "--mi-control", tConfig,
  "mi_control", "CPP_MODULES");
  tOpts.mi_exportKeyword = getOrDefault(transitionalParser, "--mi-export-keyword", tConfig,
    "mi_exportKeyword", "EXPORT");
  tOpts.mi_exportBlockBegin =  getOrDefault(transitionalParser, "--mi-export-block-begin",
    tConfig, "mi_exportBlockBegin", "BEGIN_EXPORT");
  tOpts.mi_exportBlockEnd =  getOrDefault(transitionalParser, "--mi-export-block-end",
    tConfig, "mi_exportBlockEnd", "END_EXPORT");
  tOpts.exportMacrosPath = getOrDefault(transitionalParser, "--export-macros-path",
    tConfig, "exportMacrosPath", "Export.hpp");
  /*
  log(
    "backCompatHdrs: {}\n"
    "mi_control: {}\n"
    "mi_exportKeyword: {}\n"
    "mi_exportBlockBegin: {}\n"
    "mi_exportBlockEnd: {}\n"
    "exportMacrosPath: {}\n",
    tOpts.backCompatHdrs, tOpts.mi_control, tOpts.mi_exportKeyword, tOpts.mi_exportBlockBegin,
    tOpts.mi_exportBlockEnd, tOpts.exportMacrosPath);
  */
  return opts;
}
