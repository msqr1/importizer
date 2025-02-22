#include "OptProcessor.hpp"
#include "Base.hpp"
#define ARGS_NOEXCEPT 1
#include "../3rdParty/args.hpp"
#define TOML_EXCEPTIONS 0
#define TOML_ENABLE_FORMATTERS 0
#include <toml++/toml.hpp>
#include <vector>
#include <concepts>
#include <optional>
#include <utility>
#include <filesystem>
#include <string>
#include <string_view>

namespace fs = std::filesystem;
namespace {

template <typename T>
auto getTypeCk(const toml::table& tbl, std::string_view key) {
  const toml::node_view<const toml::node> node{tbl[key]};
  if constexpr(std::is_convertible_v<T, std::string>) {
    if(node.is_string()) return node.ref<std::string>();
  }
  else if(node.is<T>()) return node.ref<T>();

  // Clang++ < 20 can't handle [[noreturn]] in constructors (eg. exitWithErr).
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic push
  exitWithErr("Incorrect TOML type for {}", key);
#pragma GCC diagnostic pop
}
template<typename T> concept ArgsFlag = requires(T flg) {
  std::derived_from<T, args::FlagBase>;
};
template<typename T, ArgsFlag F>
auto getOrDefault(F& flg, const toml::table& tbl, std::string_view key, T&& defaultVal) {
  if(flg) return flg.Get();
  else if(tbl.contains(key)) return getTypeCk<T>(tbl, key);
  else if constexpr(std::is_convertible_v<T, std::string>) return std::string{defaultVal};
  else return std::forward<T>(defaultVal);
}
template<typename T, ArgsFlag F>
auto getMustHave(F& flg, const toml::table& tbl, std::string_view key) {
  if(flg) return flg.Get();
  else if(tbl.contains(key)) return getTypeCk<T>(tbl, key);
  exitWithErr("{} must be specified", key);
}

// So much repeating...
#define GET_OR_DEFAULT(opts, opt, config, defaultVal) \
  opts.opt = getOrDefault(opt, config, #opt, defaultVal)
#define GET_MUST_HAVE(type, opts, opt) \
  opts.opt = getMustHave<type>(opt, config, #opt)
void getPathArr(args::ValueFlagList<std::string>& flg, const toml::table& tbl, 
  std::string_view key, std::vector<fs::path>& container,
  const std::optional<fs::path>& prefix = std::nullopt) {
  if(flg) for(std::string& s : flg.Get()) container.emplace_back(std::move(s));
  else if(tbl.contains(key)) {
    toml::array arr{*tbl.get(key)->as_array()};
    for(toml::node& elem : arr) {
      if(!elem.is<std::string>()) exitWithErr("{} must only contains strings", key);
      if(prefix) container.emplace_back(*prefix / std::move(elem.ref<std::string>()));
      else container.emplace_back(std::move(elem.ref<std::string>()));
    }
  }
}

} // namespace
Opts getOptsOrExit(int argc, const char** argv) {
  Opts opts;
  args::ArgumentParser parser
    {"Backward compatibly convert header-based C++ code to modules"};
  args::HelpFlag version{parser, "", 
    "Print version and exit.", 
    {'v', "version"}, args::Options::KickOut};
  args::Group general{parser, "", args::Group::Validators::DontCare,
    args::Options::Global};
  args::HelpFlag help{general, "", 
    "Print help and exit.",
    {'h', "help"}, args::Options::KickOut};
  args::Flag stdIncludeToImport{general, "",
   "Convert standard includes to 'import std' or 'import std.compat'.",
    {'s', "std-include-to-import"}};
  args::Flag logCurrentFile{general, "",
    "Print the current file being processed.",
    {'l', "log-current-file"}};
  args::Flag pragmaOnce{general, "",
    "Declare that you use '#pragma once' so importizer handles them.",
    {'p', "pragma-once"}};
  args::Flag SOFComments{general, "",
    "Declare that your files may start with comments (usually to specify a license) so "
    "importizer handles them. Note that it scans for the largest continuous SOF comment "
    "chain.",
    {'S', "SOF-comments"}};
  args::ValueFlag<std::string> includeGuard{general, "",
      "Declare that you use include guards so the tool handles them. You will provide a "
      "regex to match match the entire guard, for example: '[^\\s]+_HPP'.",
      {"include-guard"}};
  args::ValueFlag<std::string> configCmd{general, "", 
    "Path to TOML configuration file (.toml), defaults to 'importizer.toml'.",
    {'c', "config"}, "importizer.toml"};
  args::ValueFlag<std::string> inDir{general, "", 
    "Input directory (must be here or in the TOML file).",
    {'i', "in-dir"}};
  args::ValueFlag<std::string> outDir{general, "", 
    "Output directory (must be here or in the TOML file).",
    {'o', "out-dir"}};
  args::ValueFlag<std::string> hdrExt{general, "", 
    "Header file extension.",
    {"hdr-ext"}};
  args::ValueFlag<std::string> srcExt{general, "", 
    "Source (also module implementation unit) file extension.",
    {"src-ext"}};
  args::ValueFlag<std::string> moduleInterfaceExt{general, "", 
    "Module interface unit file extension.",
    {"module-interface-ext"}};
  args::ValueFlagList<std::string> includePaths{general, "",
    "Include paths searched when converting include to import.",
    {"include-paths"}};
  args::ValueFlagList<std::string> ignoredHdrs{general, "",
    "Paths relative to [inDir] of header files to ignore. Their paired sources, if "
    "available, will be treated as if they have a 'main()'.",
    {"ignored-hdrs"}};
  args::ValueFlagList<std::string> umbrellaHdrs{general, "",
    "Paths relative to [inDir] of modularized headers, but their 'import' are turned "
    "into 'export import'.",
    {"umbrella-hdrs"}};
  args::Command transitional{parser, "transitional",
    "Turn on transitional mode"};
  args::Flag backCompatHdrs{transitional, "",
    "Generate headers that include the module file to preserve #include for users. Note "
    "that in the project itself the module file is still included directly.",
    {'b', "back-compat-hdrs"}};
  args::ValueFlag<std::string> mi_control{transitional, "", 
    "Header-module switching macro identifier.",
    {"mi-control"}};
  args::ValueFlag<std::string> mi_exportKeyword{transitional, "", 
    "Exported symbol macro identifier.",
    {"mi-export-keyword"}};
  args::ValueFlag<std::string> mi_exportBlockBegin{transitional, "", 
    "Export block begin macro identifier.",
    {"mi-export-block-begin"}};
  args::ValueFlag<std::string> mi_exportBlockEnd{transitional, "", 
    "Export block end macro identifier.",
    {"mi-export-block-end"}};
  args::ValueFlag<std::string> exportMacrosPath{transitional, "", 
    "File path relative to outDir to store the export macros above.",
    {"export-macros-path"}};
  if(!parser.ParseCLI(argc, argv)) exitWithErr("{}", parser.GetErrorMsg());
  if(version) {
    println("{}", "2.0.0");
    exitOK();
  }
  if(help) {
    print("{}", parser.Help());
    exitOK();  
  }
  const fs::path configPath{std::move(*configCmd)};
  const fs::path configDir{configPath.parent_path()};
  toml::parse_result parseRes{toml::parse_file(configPath.native())};
  if(!parseRes) {
    toml::parse_error err{std::move(parseRes.error())};
    exitWithErr("TOML++ error: {} at {}({}:{})", err.description(), configPath,
    err.source().begin.line, err.source().begin.column);
  }
  const toml::table config{std::move(parseRes.table())};
  GET_OR_DEFAULT(opts, stdIncludeToImport, config, false);
  GET_OR_DEFAULT(opts, logCurrentFile, config, false);
  GET_OR_DEFAULT(opts, pragmaOnce, config, false);
  GET_OR_DEFAULT(opts, SOFComments, config, false);
  if(!(includeGuard || config.contains("includeGuard"))) opts.includeGuard = std::nullopt;
  
  // A slight hack using getMustHave even though the error case is never reached. I don't
  // want to write more code lol
  else opts.includeGuard.emplace(format(
    "^{}$",
    getMustHave<std::string>(includeGuard, config, "includeGuard")));
  GET_MUST_HAVE(std::string, opts, inDir);
  if(!inDir) opts.inDir = configDir / opts.inDir;
  GET_MUST_HAVE(std::string, opts, outDir);
  if(!outDir) opts.outDir = configDir / opts.outDir;
  GET_OR_DEFAULT(opts, hdrExt, config, ".hpp");
  GET_OR_DEFAULT(opts, srcExt, config, ".cpp");
  GET_OR_DEFAULT(opts, moduleInterfaceExt, config, ".ixx");
  getPathArr(includePaths, config, "includePaths", opts.includePaths, configDir);
  getPathArr(ignoredHdrs, config, "ignoredHdrs", opts.ignoredHdrs);
  getPathArr(umbrellaHdrs, config, "umbrellaHdrs", opts.umbrellaHdrs);
  if(!(transitional || config.contains("transitional"))) {
    opts.transitional = std::nullopt;
    return opts;
  }
  TransitionalOpts& tOpts{opts.transitional.emplace()};
  const toml::table tConfig{getTypeCk<toml::table>(config, "transitional")};
  GET_OR_DEFAULT(tOpts, backCompatHdrs, tConfig, false);
  GET_OR_DEFAULT(tOpts, mi_exportKeyword, tConfig, "EXPORT");
  GET_OR_DEFAULT(tOpts, mi_control, tConfig, "CPP_MODULES");
  GET_OR_DEFAULT(tOpts, mi_exportBlockBegin, tConfig, "BEGIN_EXPORT");
  GET_OR_DEFAULT(tOpts, mi_exportBlockEnd, tConfig, "END_EXPORT");
  GET_OR_DEFAULT(tOpts, exportMacrosPath, tConfig, "Export.hpp");
  return opts;
}