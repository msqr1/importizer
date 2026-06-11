#!/usr/bin/env bash

# Setup the development environment for Linux

if ! (return 0 2>/dev/null); then
  echo "Script must be sourced. Do '. {script}' instead of '{script}'." >&2
  exit 1
fi

# OS detection if not provided
arch=${1:-$(uname -m)}
case $arch in
amd64 | x86_64)
  arch="amd64"
  ;;
arm64 | aarch64)
  arch="arm64"
  ;;
*)
  echo "Unsupported architecture '$arch', only 'amd64' or 'arm64' is supported" >&2
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

if ! command -v clang > /dev/null 2>&1; then
  echo "Clang not found" >&2
  return 1
fi

# We are in CI
if [ -n "$CI" ]; then
  if ! command -v gh > /dev/null 2>&1; then
    echo "Github CLI not found" >&2
    return 1
  fi

  {
    echo IMPORTIZER_OS=linux
    echo "IMPORTIZER_ARCH=$arch"
  } >>"$GITHUB_ENV"
fi

export IMPORTIZER_OS=linux
export IMPORTIZER_ARCH=$arch
