set -e
scriptDir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
llvmSrc="$scriptDir/llvm-src"
3rdPartyDir=$(realpath "$scriptDir/../../3rd-party")
cmake -S "$llvmSrc/llvm" \
  -B "$llvmSrc/build" \
  -G Ninja \
  -C "$scriptDir/Cache.cmake"
cmake --build "$llvmSrc/build"
cmake --install "$llvmSrc/build" --prefix "$3rdPartyDir/llvm"
cmake -S "$llvmSrc/clang" \
  -B "$llvmSrc/build2" \
  -G Ninja \
  -C "$scriptDir/Cache.cmake" \
  -DLLVM_DIR="$3rdPartyDir/llvm/lib/cmake/llvm"
cmake --build "$llvmSrc/build2"
cmake --install "$llvmSrc/build2" --prefix "$3rdPartyDir/clang"
rm -rf "$llvmSrc"
