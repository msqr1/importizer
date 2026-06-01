$ErrorActionPreference = "Stop"

# Backslashes will cause CMake problems
$root = [System.IO.Path]::GetFullPath([System.IO.Path]::Combine($PSScriptRoot, "../..")).Replace('\', '/')

$llvmSrc = "$root/scripts/libtooling/llvm-src"
$3rdPartyDir = "$root/3rd-party"
$procCnt = [Environment]::ProcessorCount

function checkErr {
  if ($LASTEXITCODE -ne 0) {
    [System.Environment]::Exit(1)
  }
}

[System.IO.Directory]::CreateDirectory($3rdPartyDir) | Out-Null

cmake -S "$llvmSrc/llvm" -B "$llvmSrc/build" --preset libtooling-windows `
  --no-warn-unused-cli
checkErr
cmake --build "$llvmSrc/build"
checkErr
cmake --install "$llvmSrc/build" --prefix "$3rdPartyDir/llvm" -j $procCnt
checkErr

cmake -S "$llvmSrc/clang" -B "$llvmSrc/build2" --preset libtooling-windows `
  -DLLVM_DIR="$3rdPartyDir/llvm/lib/cmake/llvm" --no-warn-unused-cli
checkErr
cmake --build "$llvmSrc/build2"
checkErr
cmake --install "$llvmSrc/build2" --prefix "$3rdPartyDir/clang" -j $procCnt
checkErr

$opts = New-Object System.Diagnostics.ProcessStartInfo
$opts.UseShellExecute = $false
$opts.CreateNoWindow = $true

$opts.FileName = [System.Environment]::GetEnvironmentVariable("ComSpec")
$opts.Arguments = "/c rmdir /s /q `"$llvmSrc`""
$proc = [System.Diagnostics.Process]::Start($opts)
$proc.WaitForExit()
