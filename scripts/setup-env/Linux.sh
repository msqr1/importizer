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
esac

ASAN_OPTIONS=$([ "$arch" = "x64" ] && echo "detect_leaks=1")

export ASAN_OPTIONS="$ASAN_OPTIONS"

# We are in Github CI
if [ -n "$GITHUB_ENV" ]; then
  echo "ASAN_OPTIONS=$ASAN_OPTIONS" >> "$GITHUB_ENV"
fi
