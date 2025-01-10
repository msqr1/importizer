#include "ArgProcessor.hpp"
#include "Base.hpp"
#include "../3rdParty/Argparse.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <toml++/impl/array.hpp>
#include <toml++/impl/node.hpp>
#include <toml++/impl/node_view.hpp>
#include <toml++/impl/parse_error.hpp>
#include <toml++/impl/parse_result.hpp>
#include <toml++/impl/parser.hpp>
#include <toml++/impl/source_region.hpp>
#include <toml++/impl/table.hpp>
#include <utility>
#include <vector>

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
Opts getOptsOrExit(int argc, const char* const* argv, bool& verbose) {
  Opts opts;
  ap::ArgumentParser parser("include2import", "0.0.1");
  parser.add_description("C++ include to import converter. Takes you on the way of modularization!");
  parser.add_argument("-c", "--config")
    .help("Path to a configuration file")
    .default_value("include2import.toml");
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
  opts.inDir = configDir / getMustHave<std::string>(config, "inDir");
  opts.outDir = configDir / getMustHave<std::string>(config, "outDir");
  verbose = getOrDefault(config, "verbose", false);
  opts.stdInclude2Import = getOrDefault(config, "stdInclude2Import", false);
  opts.hdrExt = getOrDefault(config, "hdrExt", ".hpp");
  opts.srcExt = getOrDefault(config, "srcExt", ".cpp");
  opts.moduleInterfaceExt = getOrDefault(config, "moduleInterfaceExt", ".cppm");
  opts.includeGuardPat.reset(getOrDefault(config, "includeGuardPat", R"([^\s]+_H)"));
  auto getPathArr = [&](std::string_view key,
    std::vector<fs::path>& container, const fs::path& relativeTo) -> void {
    if(!config.contains(key)) return;
    const toml::array arr{*config[key].as_array()};
    for(const toml::node& elem : arr) {
      if(!elem.is<std::string>()) exitWithErr("{} must only contains strings", key);
     container.emplace_back(relativeTo / elem.ref<std::string>());
    }
  };
  getPathArr("includePaths", opts.includePaths, configDir);
  getPathArr("ignoredHeaders", opts.ignoredHeaders, configDir);
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
  return opts;
}
