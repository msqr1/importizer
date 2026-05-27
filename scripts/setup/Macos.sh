set -e
HOMEBREW_NO_ENV_HINTS=1 brew install llvm@22
export PATH="$(brew --prefix llvm@22)/bin:$PATH"
export CMAKE_PREFIX_PATH="$(brew --prefix llvm@22)"

symbolizer="$(brew --prefix llvm@22)/bin/llvm-symbolizer"
echo "$symbolizer $([ -f "$symbolizer" ] && echo "exist" || echo "doesn't exist")s"

sudo ln -sf /usr/bin/atos "$symbolizer"

# Fix external symbolizer not found error
export ASAN_OPTIONS="external_symbolizer_path=$symbolizer"
