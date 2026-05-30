param (
  [Parameter(Position = 0, Mandatory = $true)][string]$arch
)
$ErrorActionPreference = "Stop"
. "$PSScriptRoot/../FindVs.ps1" $arch
$env:CC = "clang-cl"
$env:CXX = "clang-cl"
$env:LDFLAGS = "-fuse-ld=lld-link"
$env:CMAKE_GENERATOR = "Ninja"
