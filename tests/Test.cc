#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/std.h" // IWYU pragma: keep for formatting fs::path
#include "subprocess.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

// No newlines added
template <typename... T> void err(fmt::format_string<T...> fmt, T &&...args) {
  fmt::print(
      stderr, "test: {}",
      fmt::styled("error:", fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold));
  fmt::print(stderr, fmt::emphasis::bold, fmt, std::forward<T>(args)...);
}

bool readToStr(std::FILE *f, std::string &s, std::string_view name) {
  std::fseek(f, 0, SEEK_END);
  const long len{std::ftell(f)};
  if (len == -1) {
    err("Unable to get size of {}\n", name);
    return false;
  }
#ifdef __cpp_lib_string_resize_and_overwrite
  s.resize_and_overwrite(len,
                         [len]([[maybe_unused]] char *_,
                               [[maybe_unused]] size_t _1) { return len; });
#else
  s.resize(len);
#endif
  std::fread(s.data(), 1, len, f);
  if (std::ferror(f)) {
    err("Error occured while reading {}\n", name);
    return false;
  }
}

// test [importizer path] [testDir] [outDir]
int main(const int argc, const char **argv) {
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
    err("importizer failed to start\n");
    return EXIT_FAILURE;
  }
  int exitCode;
  res = subprocess_join(&proc, &exitCode);
  if (res != 0) {
    err("Unable to wait for importizer\n");
    return EXIT_FAILURE;
  }
  std::string outCli;
  std::FILE *procStderr{subprocess_stderr(&proc)};
}
