param (
  [Parameter(Position = 0, Mandatory = $true)][string]$arch
)
$ErrorActionPreference = "Stop"
$vsPrefix = "C:/Program Files/Microsoft Visual Studio/18/Enterprise"
Import-Module "$vsPrefix/Common7/Tools/Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath "$vsPrefix" -Arch "$arch" -HostArch "$arch"

$env:CC = "clang-cl"
$env:CXX = "clang-cl"
$env:LDFLAGS = "-fuse-ld=lld-link"
$env:CMAKE_GENERATOR = "Ninja"
