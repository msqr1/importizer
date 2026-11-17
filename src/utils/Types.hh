#pragma once

struct MvOnly {
  MvOnly() noexcept = default;
  MvOnly(MvOnly &&) noexcept = default;
  auto operator=(MvOnly &&) noexcept -> MvOnly & = default;

  MvOnly(const MvOnly &) = delete;
  auto operator=(const MvOnly &) -> MvOnly & = delete;
};
