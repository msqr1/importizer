#include "ArgProcessor.hpp"
#include "Base.hpp"
#include "../3rdParty/Argparse.hpp"
#include <string_view>

namespace fs = std::filesystem;
namespace ap = argparse;

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
  auto getTypeCk = [&]<typename keyTp>(std::string_view key) {
    const toml::node_view<const toml::node> node{config[key]};

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
  }; 
  auto getOrDefault = [&]<typename T>(std::string_view key, T&& defaultVal) {
    if(config.contains(key)) return getTypeCk.template operator()<T>(key);
    if constexpr(std::is_convertible_v<T, std::string>) return std::string{defaultVal};
    else return std::forward<T>(defaultVal);
  };
  auto getStr = [&](std::string_view key) {
    if(!config.contains(key)) exitWithErr("{} must be specified", key);
    return getTypeCk.template operator()<std::string>(key);
  };
  const fs::path configDir{configPath.parent_path()};
  opts.inDir = configDir / getStr("inDir");
  opts.outDir = configDir / getStr("outDir");
  verbose = getOrDefault("verbose", false);
  opts.stdInclude2Import = getOrDefault("stdInclude2Import", false);
  opts.hdrExt = getOrDefault("hdrExt", ".hpp");
  opts.srcExt = getOrDefault("srcExt", ".cpp");
  opts.moduleInterfaceExt = getOrDefault("moduleInterfaceExt", ".cppm");
  opts.includeGuardPat.reset(getOrDefault("includeGuardPat", R"([^\s]+_H)"));
  auto getPathArr = [&](std::string_view key,
    std::vector<fs::path>& container, const fs::path& relativeTo) -> void {
    if(!config.contains(key)) return;
    const toml::array arr{*config.get_as<toml::array>(key)};
    for(const toml::node& elem : arr) {
      if(!elem.is<std::string>()) exitWithErr("{} must only contains strings", key);
     container.emplace_back(relativeTo / elem.ref<std::string>());
    }
  };
  getPathArr("includePaths", opts.includePaths, configDir);
  getPathArr("ignoredHeaders", opts.ignoredHeaders, configDir);
  return opts;
}
