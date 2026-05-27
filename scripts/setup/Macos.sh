set -e
brew install llvm@22
export PATH="$(brew --prefix llvm@22)/bin:$PATH"
export CMAKE_PREFIX_PATH="$(brew --prefix llvm@22)"
