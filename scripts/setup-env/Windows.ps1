param (
  [Parameter(Position = 0, Mandatory = $false)]
  [ValidateSet("x64", "arm64")]
  [string]$arch
)

if (!$PSBoundParameters.ContainsKey("arch")) {
  $osArch = [System.Environment]::GetEnvironmentVariable("PROCESSOR_ARCHITEW6432")
  if ([System.String]::IsNullOrEmpty($osArch)) {
    $osArch = [System.Environment]::GetEnvironmentVariable("PROCESSOR_ARCHITECTURE")
  }
  if ([System.Text.RegularExpressions.Regex]::IsMatch($osArch, "arm64", [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)) {
    $arch = "arm64"
  } elseif ([System.Text.RegularExpressions.Regex]::IsMatch($osArch, "amd64", [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)) {
    $arch = "x64"
  } else {
    [System.Console]::Error.WriteLine("Unsupported architecture '$osArch', only x64 or arm64 is supported")
    [System.Environment]::Exit(1)
  }
}

$opts = New-Object System.Diagnostics.ProcessStartInfo
$opts.RedirectStandardOutput = $true
$opts.UseShellExecute = $false
$opts.CreateNoWindow = $true

$vswhere = [System.IO.Path]::Combine(`
  [System.Environment]::GetEnvironmentVariable("ProgramFiles(x86)"),
  "Microsoft Visual Studio", "Installer", "vswhere.exe")
if (-not [System.IO.File]::Exists($vswhere)) {
  [System.Console]::Error.WriteLine("vswhere.exe not found. Visual Studio 2022 does not appear to be installed.")
  [System.Environment]::Exit(1)
}
$x = if ($arch.Equals("x64", [System.StringComparison]::OrdinalIgnoreCase)) { "x86.x64" } else { "ARM64" }
$components = @(
  "Microsoft.VisualStudio.Component.VC.Llvm.Clang",
  "Microsoft.VisualStudio.Component.Windows11SDK.26100",
  "Microsoft.VisualStudio.Component.VC.Tools.$x"
)
$vswhereArgs = "-property installationPath -version `"[17.4,)`" -latest"
foreach ($c in $components) { $vswhereArgs += " -requires $c" }
$opts.FileName = $vswhere
$opts.Arguments = $vswhereArgs
$proc = [System.Diagnostics.Process]::Start($opts)
$vsPrefix = $proc.StandardOutput.ReadToEnd().Trim()
$proc.WaitForExit()
if ($proc.ExitCode -ne 0 -or [System.String]::IsNullOrEmpty($vsPrefix)) {
  [System.Console]::Error.WriteLine("No suitable Visual Studio installation found. One must have these components:")
  foreach ($c in $components) { [System.Console]::Error.WriteLine("  - $c") }
  [System.Environment]::Exit(1)
}

$opts.FileName = [System.Environment]::GetEnvironmentVariable("ComSpec") # cmd.exe
$opts.Arguments = "/c `"`"$vsPrefix\Common7\Tools\VsDevCmd.bat`" -arch=$arch -host_arch=$arch >NUL 2>&1 && set`""
$proc = [System.Diagnostics.Process]::Start($opts)
$envOutput = $proc.StandardOutput.ReadToEnd()
$proc.WaitForExit()
if ($proc.ExitCode -ne 0) {
  [System.Console]::Error.WriteLine("Failed to start Visual Studio Developer Command Prompt")
  [System.Environment]::Exit(1)
}

$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$rawOldPath = [System.Environment]::GetEnvironmentVariable("PATH")
$oldPath = if ([System.String]::IsNullOrEmpty($rawOldPath)) { @() } else { $rawOldPath.Split(';') }
$ghEnv = [System.Environment]::GetEnvironmentVariable("GITHUB_ENV")
$ghPath = [System.Environment]::GetEnvironmentVariable("GITHUB_PATH")
$ghCI = -not [System.String]::IsNullOrEmpty($ghEnv)
$envLines = $envOutput.Split([System.Environment]::NewLine, [System.StringSplitOptions]::RemoveEmptyEntries)
$regex = New-Object System.Text.RegularExpressions.Regex("^([^=]+)=(.*)$")
foreach ($line in $envLines) {
  $match = $regex.Match($line)
  if ($match.Success) {
    $name = $match.Groups[1].Value
    $val = $match.Groups[2].Value
    [System.Environment]::SetEnvironmentVariable($name, $val)
    if ($ghCI) {
      if ($name.Equals("Path", [System.StringComparison]::OrdinalIgnoreCase)) {
        $splitVals = $val.Split(';', [System.StringSplitOptions]::RemoveEmptyEntries)
        [string[]]$newPaths = @()
        foreach ($p in $splitVals) {
          $trimmed = $p.Trim()
          if ($trimmed.Length -eq 0) { continue }
          $found = $false
          foreach ($op in $oldPath) {
            if ($op.Equals($trimmed, [System.StringComparison]::OrdinalIgnoreCase)) {
              $found = $true
              break
            }
          }
          if (-not $found) {
            $newPaths.Add($trimmed)
          }
        }
        if ($newPaths.Count -gt 0) {
          [System.IO.File]::AppendAllLines($ghPath, $newPaths, $utf8NoBom)
        }
      } else {
        [System.IO.File]::AppendAllLines($ghEnv, [string[]]"$name=$val", $utf8NoBom)
      }
    }
  }
}
