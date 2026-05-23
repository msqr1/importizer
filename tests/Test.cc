#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include "fmt/std.h" // IWYU pragma: keep for formatting fs::path
#include "subprocess.h"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
template <typename... T>
[[noreturn]] void exitWithErr(fmt::format_string<T...> f, T &&...args) {
  fmt::println(
      stderr, "test: {} {}",
      fmt::styled("error:", fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold),
      fmt::format(fmt::emphasis::bold, f, std::forward<T>(args)...));
  throw EXIT_FAILURE;
}

// test [importizer path] [testDir] [outDir]
int main(const int argc, const char **argv) {
  try {
    fs::path config{argv[2]};
    config /= "Config.toml";
    const std::string configStr{config.string()};
    std::array<const char *, 4> cmd{argv[1], configStr.c_str(), "-o", argv[3]};
    subprocess_s proc;
    int res{subprocess_create(cmd.data(),
                              subprocess_option_no_window |
                                  subprocess_option_combined_stdout_stderr,
                              &proc)};
    if (res != 0) {
      exitWithErr("importizer failed to start");
    }
    int exitCode;
    res = subprocess_join(&proc, &exitCode);
    if (res != 0) {
      exitWithErr("Unable to wait for importizer");
    }

  } catch (int code) {
    return code;
  }
  return EXIT_SUCCESS;
}
