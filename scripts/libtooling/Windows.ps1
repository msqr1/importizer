$ErrorActionPreference = "Stop"
$llvmSrc = "$PSScriptRoot/llvm-src"
$3rdPartyDir = "$PSScriptRoot/../../3rd-party"
New-Item -ItemType "Directory" -Path "$3rdPartyDir" -Force | Out-Null

# Relative path and backslash will cause CMake issues
$3rdPartyDir = (Resolve-Path $3rdPartyDir).Path.Replace('\','/')

cmake -S "$llvmSrc/llvm" `
  -B "$llvmSrc/build" `
  -G Ninja `
  -C "$PSScriptRoot/Cache.cmake" `
  -DLLVM_ENABLE_RPMALLOC=on `
  -DCMAKE_C_COMPILER=clang-cl `
  -DCMAKE_CXX_COMPILER=clang-cl `
  -DCMAKE_LINKER_TYPE=LLD
cmake --build "$llvmSrc/build"
cmake --install "$llvmSrc/build" --prefix "$3rdPartyDir/llvm"
cmake -S "$llvmSrc/clang" `
  -B "$llvmSrc/build2" `
  -G Ninja `
  -C "$PSScriptRoot/Cache.cmake" `
  -DLLVM_DIR="$3rdPartyDir/llvm/lib/cmake/llvm" `
  -DCMAKE_C_COMPILER=clang-cl `
  -DCMAKE_CXX_COMPILER=clang-cl `
  -DCMAKE_LINKER_TYPE=LLD
cmake --build "$llvmSrc/build2"
cmake --install "$llvmSrc/build2" --prefix "$3rdPartyDir/clang"
Remove-Item -Path "$llvmSrc" -Recurse -Force
