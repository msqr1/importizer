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

# Can't use Launch-VsDevShell on ARM because of
# https://developercommunity.visualstudio.com/t/Launch-VsDevShellps1-does-not-allow-arm/10740584
$envLines = & cmd.exe /c "`"$vsPrefix/Common7/Tools/VsDevCmd.bat`" -arch=$arch -host_arch=$arch >NUL 2>&1 && set"
foreach ($line in $envLines) {
  if ($line -match "^([^=]+)=(.*)$") {
    $name = $matches[1]
    $val = $matches[2]
    [System.Environment]::SetEnvironmentVariable($name, $val)
  }
}
