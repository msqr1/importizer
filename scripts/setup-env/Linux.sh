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
