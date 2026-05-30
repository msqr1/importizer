# Helper script to locate, check requirements, and bring Visual Studio tools into PATH
param (
  [Parameter(Position = 0, Mandatory = $true)]
  [ValidateSet("x64", "arm64")]
  [string]$arch
)
$ErrorActionPreference = "Stop"
$vswhere = "${Env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
$x = If ("$arch" -eq "x64") {"x86.x64"} Else {"ARM64"}
$components = @(
  "Microsoft.VisualStudio.Component.VC.Llvm.Clang"
  "Microsoft.VisualStudio.Component.Windows11SDK.26100"
  "Microsoft.VisualStudio.Component.VC.Tools.$x"
)

# Min version that support ARM64 builds
$vsInfo = & "$vswhere" -format json -utf8 -version "[17.4,)" -latest -requires $components `
  | ConvertFrom-Json
if ($vsInfo.Length -lt 1) {
  throw "No suitable Visual Studio installation found"
}
$vsPrefix = $vsInfo[0].installationPath
Import-Module "$vsPrefix/Common7/Tools/Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath "$vsPrefix" -Arch "$arch" -HostArch "$arch"
