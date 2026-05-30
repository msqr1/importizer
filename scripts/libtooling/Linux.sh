set -e
scriptDir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
llvmSrc="$scriptDir/llvm-src"
thirdPartyDir="$scriptDir/../../3rd-party"
procCnt=$(cmake -P "$scriptDir/../Nproc.cmake")
mkdir -p "$thirdPartyDir"
thirdPartyDir=$(realpath "$thirdPartyDir")
cmake -S "$llvmSrc/llvm" -B "$llvmSrc/build" --preset libtooling-linux \
  --no-warn-unused-cli
cmake --build "$llvmSrc/build" -j $procCnt
cmake --install "$llvmSrc/build" --prefix "$thirdPartyDir/llvm" -j $procCnt
cmake -S "$llvmSrc/clang" -B "$llvmSrc/build2" --preset libtooling-linux \
  -DLLVM_DIR="$thirdPartyDir/llvm/lib/cmake/llvm" --no-warn-unused-cli
cmake --build "$llvmSrc/build2" -j $procCnt
cmake --install "$llvmSrc/build2" --prefix "$thirdPartyDir/clang" -j $procCnt
rm -rf "$llvmSrc"
