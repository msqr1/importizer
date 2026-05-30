set -e
scriptDir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
llvmSrc="$scriptDir/llvm-src"
thirdPartyDir="$scriptDir/../../3rd-party"
mkdir -p "$thirdPartyDir"
thirdPartyDir=$(realpath "$thirdPartyDir")
cmake -S "$llvmSrc/llvm" \
  -B "$llvmSrc/build" \
  --preset libtooling-macos \
  --no-warn-unused-cli
cmake --build "$llvmSrc/build"
cmake --install "$llvmSrc/build" --prefix "$thirdPartyDir/llvm"
cmake -S "$llvmSrc/clang" \
  -B "$llvmSrc/build2" \
  -DLLVM_DIR="$thirdPartyDir/llvm/lib/cmake/llvm" \
  --preset libtooling-macos \
  --no-warn-unused-cli
cmake --build "$llvmSrc/build2"
cmake --install "$llvmSrc/build2" --prefix "$thirdPartyDir/clang"
rm -rf "$llvmSrc"
