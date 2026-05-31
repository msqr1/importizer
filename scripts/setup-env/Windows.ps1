param (
  [Parameter(Position = 0, Mandatory = $false)]
  [ValidateSet("x64", "arm64")]
  [string]$arch
)
if (!$PSBoundParameters.ContainsKey("arch")) {
  if ($env:PROCESSOR_ARCHITEW6432) {
    $osArch = $env:PROCESSOR_ARCHITEW6432
  } else {
    $osArch = $env:PROCESSOR_ARCHITECTURE
  }
  switch -regex ($osArch) {
    "(i?)arm64" { $arch = "arm64" }
    "(i?)amd64" { $arch = "x64" }
    default {
      Write-Error "Unsupported architecture '$osArch', only x64 or arm64 is supported"
      exit 1
    }
  }
}
$vswhere = "${Env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
if (!(Test-Path "$vswhere")) {
  Write-Error "vswhere.exe not found. Visual Studio 2022 does not appear to be installed."
  Exit 1
}
$x = If ("$arch" -eq "x64") {"x86.x64"} Else {"ARM64"}
$components = @(
  "Microsoft.VisualStudio.Component.VC.Llvm.Clang",
  "Microsoft.VisualStudio.Component.Windows11SDK.26100",
  "Microsoft.VisualStudio.Component.VC.Tools.$x"
)
$vsInfoStr = & "$vswhere" -format json -utf8 -version "[17.4,)" -latest -requires $components
if (!$?) {
  Write-Error "No suitable Visual Studio installation found. One must have these components: "
  foreach ($c in $components) {
    Write-Error "$c"
  }
  Exit 1
}
$vsInfo = $vsInfoStr | ConvertFrom-Json
$vsPrefix = $vsInfo[0].installationPath
$envLines = & cmd.exe /c "`"$vsPrefix/Common7/Tools/VsDevCmd.bat`" -arch=$arch -host_arch=$arch >NUL 2>&1 && set"
if (!$?) {
  Write-Error "Failed to start Visual Studio Developer Command Prompt"
  Exit 1
}
foreach ($line in $envLines) {
  if ($line -match "^([^=]+)=(.*)$") {
    $name = $matches[1]
    $val = $matches[2]
    [System.Environment]::SetEnvironmentVariable($name, $val)
    if ($env:GITHUB_ENV) {
      if ($name -eq "Path" -or $name -eq "PATH") {
        $val -split ';' | Where-Object { $_.Trim() -ne "" } | Out-File -FilePath $env:GITHUB_PATH -Encoding utf8 -Append
      } else {
        "$name=$val" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
      }
    }
  }
}
