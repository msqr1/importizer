#include "run-test/CmpDir.hh"
#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <array>
#include <cassert>
#include <cstdio> // IWYU pragma: keep for stderr
#include <cstdlib>
#include <filesystem>
#include <fmt/base.h>
#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>

namespace fs = std::filesystem;
namespace rp = reproc;

const std::string_view prog{"test"};

// test [importizer path] [testDir] [outDir]
// Expecting absolute paths & no input validation
int main(const int argc, const char **argv) {
  assert(argc == 4);

  if (!getRaw(argc, argv)) {
    return EXIT_FAILURE;
  };

  const fs::path testDir{argv[2]};
  const fs::path outDir{argv[3]};
  const std::string config{(testDir / "Config.toml").string()};
  const std::array<std::string_view, 4> cmd{argv[1], config, "-o", argv[3]};
  rp::process proc;
  rp::options opts;

  // Set this to drain(), somehow you don't need this for stdout
  opts.redirect.err.type = rp::redirect::type::pipe;

  std::error_code errCode{proc.start(cmd, opts)};
  if (errCode) [[unlikely]] {
    err("Unable to start {}: {}", cmd, errCode.message());
    return EXIT_FAILURE;
  }

  std::string out;
  errCode = rp::drain(proc, rp::sink::null, rp::sink::string{out});
  if (errCode) [[unlikely]] {
    err("Unable to read output of {}: {}", cmd, errCode.message());
    return EXIT_FAILURE;
  }

  int rtn;
  std::tie(rtn, errCode) = proc.wait(rp::infinite);
  if (errCode) [[unlikely]] {
    err("Unable to wait for {}: {}", cmd, errCode.message());
    return EXIT_FAILURE;
  }

#ifdef _WIN32
  std::erase(out, '\r');
#endif
  std::string ref;
  if (!(readToStr(testDir / "RefCli.txt", ref))) [[unlikely]] {
    return EXIT_FAILURE;
  }

  bool errored{};

  const int refRtn{ref.contains("importizer: error: ") ? EXIT_FAILURE
                                                       : EXIT_SUCCESS};
  if ((errored |= refRtn != rtn)) {
    err("Mismatched return code: expected {}, got {}", refRtn, rtn);
  }

  if ((errored |= out != ref)) {
    err("Mismatched CLI output, got");

    // Don't format importizer's output
    fmt::print(stderr, "{}", out);
  }

  fs::path refDir{testDir / "ref"};
  errored |= fs::exists(refDir) && !cmpDir(outDir, refDir);

  return errored ? EXIT_FAILURE : EXIT_SUCCESS;
}
