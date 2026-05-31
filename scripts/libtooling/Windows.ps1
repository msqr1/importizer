$ErrorActionPreference = "Stop"

# Relative path and backslash will cause CMake issues
$root=(Resolve-Path $PSScriptRoot/../..).Path.Replace('\','/')

$llvmSrc = "$root/scripts/libtooling/llvm-src"
$3rdPartyDir = "$root/3rd-party"
New-Item -ItemType "Directory" -Path "$3rdPartyDir" -Force | Out-Null
function checkErr {
  if(!$?) {
    Exit 1
  }
}
$procCnt = cmake -P "$root/scripts/Nproc.cmake"
checkErr
cmake -S "$llvmSrc/llvm" -B "$llvmSrc/build" --preset libtooling-windows `
  --no-warn-unused-cli
checkErr
cmake --build "$llvmSrc/build" -j $procCnt
checkErr
cmake --install "$llvmSrc/build" --prefix "$3rdPartyDir/llvm" -j $procCnt
checkErr
cmake -S "$llvmSrc/clang" -B "$llvmSrc/build2" --preset libtooling-windows `
  -DLLVM_DIR="$3rdPartyDir/llvm/lib/cmake/llvm" --no-warn-unused-cli
checkErr
cmake --build "$llvmSrc/build2" -j $procCnt
checkErr
cmake --install "$llvmSrc/build2" --prefix "$3rdPartyDir/clang" -j $procCnt
checkErr
Remove-Item -Path "$llvmSrc" -Recurse -Force
