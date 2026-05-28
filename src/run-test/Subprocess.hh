#pragma once

#include "utils/Log.hh"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdio>
#include <fmt/format.h>
#include <optional>
#include <string>
#include <subprocess.h>
#include <system_error>
#include <utility>
#ifdef WIN32
#include <win32/windows_base.h>
#else
#include <cerrno>
#endif

// RAII + more convenient subprocess_s
template <size_t n_arg> struct Proc : subprocess_s {
  const std::array<const char *, n_arg> &cmd;
  Proc(subprocess_s proc, const std::array<const char *, n_arg> &cmd)
      : subprocess_s{proc}, cmd{cmd} {}
  ~Proc() noexcept { subprocess_destroy(this); }

  // Disallow copying
  Proc(const Proc &) = delete;

  // Disallow moving
  Proc(Proc &&other) noexcept = delete;

  [[nodiscard]] bool getOutput(std::string &out, bool stdOut = true) noexcept {
    std::array<char, 128> chunk;
    std::FILE *stream{stdOut ? stdout_file : stderr_file};
    size_t n_read;
    while (true) {
      n_read = std::fread(chunk.data(), sizeof(char), chunk.size(), stream);
      if (std::ferror(stream)) [[unlikely]] {
        err("Unable to read {} of {}\n", stdOut ? "stdout" : "stderr", cmd);
        return false;
      }
      if (n_read > 0) {
        out.append(chunk.data(), n_read);
      } else {
        break;
      }
    }
    return true;
  }

  [[nodiscard]] std::optional<int> join() noexcept {
    int rtnCode;
    if (subprocess_join(this, &rtnCode) != 0) [[unlikely]] {
      err("Unable to wait {}\n", cmd);
      return std::nullopt;
    }
    return rtnCode;
  }
};

// Factory
template <size_t n_arg>
[[nodiscard]] std::optional<Proc<n_arg>>
startProc(const std::array<const char *, n_arg> &cmd,
          int opts = subprocess_option_no_window |
                     subprocess_option_inherit_environment) {
  subprocess_s proc;

  // Subproccess.h requires a terminating nullptr element
  std::array<const char *, n_arg + 1> cmdWithNull;
  std::ranges::copy(cmd, cmdWithNull.begin());
  int res{subprocess_create(cmdWithNull.data(), opts, &proc)};
  if (res != 0) [[unlikely]] {
#ifdef WIN32
    DWORD errCode{GetLastError()};
#else
    int errCode{errno};
#endif
    err("Failed to start {}: {}\n", cmd,
        std::system_category().message(errCode));
    return std::nullopt;
  }
  return std::optional<Proc<n_arg>>(std::in_place, proc, cmd);
}
