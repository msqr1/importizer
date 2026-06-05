arch=${1:-$(uname -m)}
case $arch in
x64 | x86_64)
  arch="x64"
  ;;
arm64 | aarch64)
  arch="arm64"
  ;;
*)
  echo "Unsupported architecture '$arch', only x64 or arm64 is supported" >&2
  return 1 2>/dev/null || exit 1
  ;;
esac

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is not installed" >&2
  return 1 2>/dev/null || exit 1
fi

v=18
llvmPrefix="$(brew --prefix llvm@$v || true)"
if [ ! -d "$(brew --prefix llvm@$v || true)" ]; then
  echo "Homebrew LLVM $v not found." >&2
  return 1 2>/dev/null || exit 1
fi

asanOpts=""
if [ "$arch" = "x64" ]; then
  asanOpts="detect_leaks=1"
fi

v=18

export ASAN_OPTIONS="$asanOpts"
export CMAKE_PREFIX_PATH="$llvmPrefix"
export IMPORTIZER_OS=macos
export IMPORTIZER_ARCH=$arch

# We are in CI
if [ -n "$CI" ]; then
  {
    echo "ASAN_OPTIONS=$asanOpts"
    echo "CMAKE_PREFIX_PATH=$llvmPrefix"
    echo IMPORTIZER_OS=macos
    echo "IMPORTIZER_ARCH=$arch"
  } >>"$GITHUB_ENV"
fi
