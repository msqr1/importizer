arch=$1
if [ -z "$arch" ]; then
  arch=$(uname -m)
  if [ "$arch" = x86_64 ]; then
    arch="x64"
  elif [[ "$arch" = arm64 || "$arch" = aarch64 ]]; then
    arch="arm64"
  else
  echo "Unsupported architecture '$arch', only x64 or arm64 is supported" >&2
  return 1 2>/dev/null || exit 1
  fi
fi
if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is not installed" >&2
  return 1 2>/dev/null || exit 1
fi
v=18
LLVM_PREFIX="$(brew --prefix llvm@$v)"
if [ ! -d "$LLVM_PREFIX" ]; then
  echo "Homebrew LLVM $v not found." >&2
  return 1 2>/dev/null || exit 1
fi
ASAN_OPTIONS=$([ "$arch" = "x64" ] && echo "detect_leaks=1" || echo "")
export PATH="${LLVM_PREFIX}/bin:$PATH"
export CMAKE_PREFIX_PATH="$LLVM_PREFIX"
export ASAN_OPTIONS="$ASAN_OPTIONS"
if [ -n "$GITHUB_ENV" ]; then
  echo "CMAKE_PREFIX_PATH=$LLVM_PREFIX" >> "$GITHUB_ENV"
  echo "ASAN_OPTIONS=$ASAN_OPTIONS" >> "$GITHUB_ENV"
fi
if [ -n "$GITHUB_PATH" ]; then
  echo "${LLVM_PREFIX}/bin" >> "$GITHUB_PATH"
fi
