#include "test/CmpDir.hh"
#include "test/Subprocess.hh"
#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/format.h>
#include <optional>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

const std::string_view prog{"test"};

// test [importizer path] [testDir] [outDir]
// Expecting absolute paths & no input validation
int main(const int argc, const char **argv) {
  // May have -r
  [[assume(argc > 3 && argc < 7)]];

  if (!getRaw(argc, argv)) {
    return EXIT_FAILURE;
  };
  const fs::path testDir{argv[2]};
  const fs::path outDir{argv[3]};
  const std::string config{(testDir / "Config.toml").string()};
  std::array<const char *, 4> cmd{argv[1], config.c_str(), "-o", argv[3]};
  auto proc{startProc(cmd)};
  if (!proc) [[unlikely]] {
    return EXIT_FAILURE;
  }
  std::optional<int> rtn{proc->join()};
  if (!rtn) [[unlikely]] {
    return EXIT_FAILURE;
  }
  std::string out;
  std::string ref;
  if (!(readToStr(testDir / "RefCli.txt", ref) && proc->getOutput(out, false)))
      [[unlikely]] {
    return EXIT_FAILURE;
  }
  bool errored{};
  const int refRtn = ref.contains("error: ") ? EXIT_FAILURE : EXIT_SUCCESS;
  if (refRtn != *rtn) {
    err("Mismatched return code: expected {}, got {}\n", refRtn, *rtn);
    errored = true;
  }
  if (out != ref) {
    err("Mismatched CLI output, got\n");

    // Don't format importizer's output
    fmt::print(stderr, "{}", out);

    errored = true;
  }
  fs::path refDir{testDir / "ref"};
  errored |= fs::exists(refDir) && !cmpDir(outDir, refDir);
  return errored ? EXIT_FAILURE : EXIT_SUCCESS;
}
