#!/usr/bin/env zsh

# Setup the development environment for MacOS

if [[ ! $ZSH_EVAL_CONTEXT =~ :file$ ]]; then
  echo "Script must be sourced. Do '. {script}' instead of '{script}'." >&2
  exit 1
fi

# OS detection if not provided
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
  return 1
  ;;
esac

if ! command -v cmake > /dev/null 2>&1; then
  echo "CMake not found" >&2
  return 1
fi

if ! command -v ninja > /dev/null 2>&1; then
  echo "Ninja not found" >&2
  return 1
fi

if ! command -v brew > /dev/null 2>&1 ; then
  echo "Homebrew is not installed" >&2
  return 1
fi

v=18
llvmPrefix="$(brew --prefix llvm@$v || true)"
if [ ! -d "$llvmPrefix" ]; then
  echo "Homebrew LLVM $v not found." >&2
  return 1
fi

export CMAKE_PREFIX_PATH="$llvmPrefix"
export IMPORTIZER_OS=macos
export IMPORTIZER_ARCH=$arch

# We are in CI
if [ -n "$CI" ]; then
  {
    echo "CMAKE_PREFIX_PATH=$llvmPrefix"
    echo IMPORTIZER_OS=macos
    echo "IMPORTIZER_ARCH=$arch"
  } >>"$GITHUB_ENV"
fi
