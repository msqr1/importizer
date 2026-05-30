set -e
v=18
export PATH="$(brew --prefix llvm@${v})/bin:$PATH"
export CMAKE_PREFIX_PATH="$(brew --prefix llvm@${v})"
