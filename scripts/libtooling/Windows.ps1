$ErrorActionPreference = "Stop"
$llvmSrc = "$PSScriptRoot/llvm-src"
$3rdPartyDir = "$PSScriptRoot/../../3rd-party"
New-Item -ItemType "Directory" -Path "$3rdPartyDir" -Force | Out-Null

# Relative path and backslash will cause CMake issues
$3rdPartyDir = (Resolve-Path $3rdPartyDir).Path.Replace('\','/')

cmake -P
cmake -S "$llvmSrc/llvm" `
  -B "$llvmSrc/build" `
  --preset libtooling-windows `
  --no-warn-unused-cli
cmake --build "$llvmSrc/build"
cmake --install "$llvmSrc/build" --prefix "$3rdPartyDir/llvm"
cmake -S "$llvmSrc/clang" `
  -B "$llvmSrc/build2" `
  -DLLVM_DIR="$3rdPartyDir/llvm/lib/cmake/llvm" `
  --preset libtooling-windows `
  --no-warn-unused-cli
cmake --build "$llvmSrc/build2"
cmake --install "$llvmSrc/build2" --prefix "$3rdPartyDir/clang"
Remove-Item -Path "$llvmSrc" -Recurse -Force
