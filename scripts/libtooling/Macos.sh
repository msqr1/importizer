set -e

root=$(realpath "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)/../..")
llvmSrc="$root/scripts/libtooling/llvm-src"
_3rdPartyDir="$root/3rd-party"
procCnt=$(sysctl -n hw.logicalcpu)

mkdir -p "$_3rdPartyDir"

cmake -S "$llvmSrc/llvm" -B "$llvmSrc/build" --preset libtooling-macos \
  --no-warn-unused-cli
cmake --build "$llvmSrc/build" -j $procCnt
cmake --install "$llvmSrc/build" --prefix "$_3rdPartyDir/llvm" -j $procCnt

cmake -S "$llvmSrc/clang" -B "$llvmSrc/build2" --preset libtooling-macos \
  -DLLVM_DIR="$_3rdPartyDir/llvm/lib/cmake/llvm" --no-warn-unused-cli
cmake --build "$llvmSrc/build2" -j $procCnt
cmake --install "$llvmSrc/build2" --prefix "$_3rdPartyDir/clang" -j $procCnt

rm -rf "$llvmSrc"
