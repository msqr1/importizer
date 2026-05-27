param (
  [Parameter(Position = 0, Mandatory = $true)]
  [ValidateSet("Community", "Enterprise")]
  [string]$vsEdition
)
$ErrorActionPreference = "Stop"
$vsPrefix = "C:\Program Files\Microsoft Visual Studio\18\$vsEdition"
Import-Module "$vsPrefix\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
Enter-VsDevShell -VsInstallPath "$vsPrefix" -Arch x64 -HostArch x64
