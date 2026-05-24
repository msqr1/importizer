#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/std.h" // IWYU pragma: keep for formatting fs::path
#include "subprocess.h"
#include "utils/FileOp.hh"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// No newlines added
template <typename... T> void err(fmt::format_string<T...> fmt, T &&...args) {
  fmt::print(
      stderr, "test: {}",
      fmt::styled("error:", fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold));
  fmt::print(stderr, fmt::emphasis::bold, fmt, std::forward<T>(args)...);
}

// test [importizer path] [testDir] [outDir]
int main(const int argc, const char **argv) {
  fs::path testDir{argv[2]};
  fs::path outDir{argv[3]};
  const std::string config{(testDir / "Config.toml").string()};
  std::array<const char *, 4> cmd{argv[1], config.c_str(), "-o", argv[3]};
  subprocess_s proc;
  int res{subprocess_create(cmd.data(),
                            subprocess_option_no_window |
                                subprocess_option_combined_stdout_stderr,
                            &proc)};
  if (res != 0) {
    err("importizer failed to start\n");
    return EXIT_FAILURE;
  }
  int exitCode;
  res = subprocess_join(&proc, &exitCode);
  if (res != 0) {
    err("Unable to wait for importizer\n");
    return EXIT_FAILURE;
  }
  std::string out;
  if (!readToStr(subprocess_stdout(&proc), out, "importizer stdout")) {
    return EXIT_FAILURE;
  };
  std::string ref;
  if (!readToStr(testDir / "RefCli.txt", ref)) {
    return EXIT_FAILURE;
  }
}
