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

asanOpts=""
if [ "$arch" = "x64" ]; then
  asanOpts="detect_leaks=1"
fi

export ASAN_OPTIONS="$asanOpts"

# We are in Github CI
if [ -n "$GITHUB_ENV" ]; then
  echo "ASAN_OPTIONS=$asanOpts" >> "$GITHUB_ENV"
fi
