#include "OptProcessor.hpp"
#include "Base.hpp"
#include "../3rdParty/Argparse.hpp"
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
auto getOrDefault(const toml::table& tbl, std::string_view key, T&& defaultVal) {
  if(tbl.contains(key)) return getTypeCk<T>(tbl, key);
  if constexpr(std::is_convertible_v<T, std::string>) return std::string{defaultVal};
  else return std::forward<T>(defaultVal);
}

template<typename T> 
auto getMustHave(const toml::table& tbl, std::string_view key) {
  if(!tbl.contains(key)) exitWithErr("{} must be specified", key);
  return getTypeCk<T>(tbl, key);
}

}

Opts getOptsOrExit(int argc, const char* const* argv) {
  Opts opts;
  ap::ArgumentParser parser("include2import", "0.0.1");
  parser.add_description("C++ include to import converter. Takes you on the way of modularization!");
  parser.add_argument("-c", "--config")
    .help("Path to a TOML configuration file")
    .default_value("include2import.toml");
  parser.add_argument("-o", "--outDir")
    .help("Output directory");
  parser.add_argument("-i", "--inDir")
    .help("Input directory");
  parser.parse_args(argc, argv);
  const fs::path configPath{parser.get("-c")};
  const toml::parse_result parseRes{toml::parse_file(configPath.native())};
  if(!parseRes) {
    const toml::parse_error err{parseRes.error()};
    const toml::source_region errSrc{err.source()};
    exitWithErr("TOML++ error: {} at {}({}:{})", err.description(), configPath.native(), 
    errSrc.begin.line, errSrc.begin.column);
  }
  const toml::table config{std::move(parseRes.table())};
  const fs::path configDir{configPath.parent_path()};
  std::optional<fs::path> path{parser.present("-i")};
  opts.inDir = path ? 
    std::move(*path) : configDir / getMustHave<std::string>(config, "inDir");
  path = parser.present("-o");
  opts.outDir = path ? 
    std::move(*path) : configDir / getMustHave<std::string>(config, "outDir");
  opts.logCurrentFile = getOrDefault(config, "logCurrentFile", false);
  opts.stdInclude2Import = getOrDefault(config, "stdInclude2Import", false);
  opts.hdrExt = getOrDefault(config, "hdrExt", ".hpp");
  opts.srcExt = getOrDefault(config, "srcExt", ".cpp");
  opts.moduleInterfaceExt = getOrDefault(config, "moduleInterfaceExt", ".cppm");
  opts.includeGuardPat.reset(getOrDefault(config, "includeGuardPat", R"([^\s]+_H)"));
  auto getPathArr = [&](std::string_view key,
    std::vector<fs::path>& container, const std::optional<fs::path>& prefix) -> void {
    if(!config.contains(key)) return;
    const toml::array arr{*config[key].as_array()};
    for(const toml::node& elem : arr) {
      if(!elem.is<std::string>()) exitWithErr("{} must only contains strings", key);
      if(prefix) container.emplace_back(*prefix / elem.ref<std::string>());
      else container.emplace_back(elem.ref<std::string>());
    }
  };
  getPathArr("includePaths", opts.includePaths, configDir);
  getPathArr("ignoredHeaders", opts.ignoredHeaders, std::nullopt);
  if(!config.contains("Transitional")) {
    opts.transitionalOpts = std::nullopt;
    return opts;
  }
  const toml::table transitionalConfig{getTypeCk<toml::table>(config, "Transitional")};
  opts.transitionalOpts.emplace();
  opts.transitionalOpts->mi_control = 
    getOrDefault(transitionalConfig, "mi_control", "CPP_MODULES");
  opts.transitionalOpts->mi_exportKeyword =
    getOrDefault(transitionalConfig, "mi_exportKeyword", "EXPORT");
  opts.transitionalOpts->mi_exportBlockBegin = 
    getOrDefault(transitionalConfig, "mi_exportBlockBegin", "BEGIN_EXPORT");
  opts.transitionalOpts->mi_exportBlockEnd = 
    getOrDefault(transitionalConfig, "mi_exportBlockEnd", "END_EXPORT");
  opts.transitionalOpts->exportMacrosPath =
    getOrDefault(transitionalConfig, "exportMacrosPath", "Export.hpp");
  return opts;
}
