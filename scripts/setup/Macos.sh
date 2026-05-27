set -e
brew install llvm@22
export PATH="$(brew --prefix llvm@22)/bin:$PATH"
export CMAKE_PREFIX_PATH="$(brew --prefix llvm@22)"

# Fix external symbolizer not found error
export ASAN_OPTIONS="external_symbolizer_path=$(brew --prefix llvm@22)/bin/llvm-symbolizer"
