# Setup the development environment for Windows

using namespace System

param (
  [Parameter(Position = 0)][ValidateSet("amd64", "arm64")][string]$arch,
  [ValidateNotNullOrEmpty()][string]$llvmPrefix = "C:/Program Files/LLVM",
  [ValidateNotNullOrEmpty()][string]$cmakePrefix = "C:/Program Files/CMake"
)

function exitWithErr {
  param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [object[]]$parts
  )
  $sb = [Text.StringBuilder]::new()
  foreach ($part in $parts) {
    if ($null -eq $part) { continue }
    if ($part -is [Collections.IEnumerable] -and $part -isnot [string]) {
      foreach ($item in $part) {
        if ($null -ne $item) {
          $sb.Append($item.ToString())
          $sb.Append(", ")
        }
      }
    } else {
      $sb.Append($part.ToString())
    }
  }
  throw [Management.Automation.RuntimeException]::new($sb.ToString())
}

if ($MyInvocation.InvocationName -ne '.') {
  exitWithErr("Script must be sourced. Do '. {script}' instead of '{script}'.")
}

# OS detection if not provided
if (!$PSBoundParameters.ContainsKey("arch")) {
  $osArch = [Environment]::GetEnvironmentVariable("PROCESSOR_ARCHITEW6432")
  if ([string]::IsNullOrEmpty($osArch)) {
    $osArch = [Environment]::GetEnvironmentVariable("PROCESSOR_ARCHITECTURE")
  }
  if ([Text.RegularExpressions.Regex]::IsMatch($osArch, "arm64",
      [Text.RegularExpressions.RegexOptions]::IgnoreCase)) {
    $arch = "arm64"
  } elseif ([Text.RegularExpressions.Regex]::IsMatch($osArch, "amd64",
      [Text.RegularExpressions.RegexOptions]::IgnoreCase)) {
    $arch = "amd64"
  } else {
    exitWithErr("Unsupported architecture '", $osArch, "', only 'amd64' or 'arm64' is supported")
  }
}

if (!(Get-Command ninja -ErrorAction SilentlyContinue)) {
  exitWithErr("Ninja not found")
}

$ci = ![string]::IsNullOrEmpty([Environment]::GetEnvironmentVariable("CI"))
$utf8NoBom = [Text.UTF8Encoding]::new($false)
$opts = [Diagnostics.ProcessStartInfo]::new()
$opts.RedirectStandardOutput = $true
$opts.UseShellExecute = $false
$opts.CreateNoWindow = $true

# Check SDK (and Clang if :bundled) with vswhere
$vswhere = [IO.Path]::Combine(
  [Environment]::GetEnvironmentVariable("ProgramFiles(x86)"), "Microsoft Visual Studio",
  "Installer", "vswhere.exe")
if (![IO.File]::Exists($vswhere)) {
  exitWithErr("vswhere not found. Visual Studio does not appear to be installed.")
}
$x = if ($arch.Equals("amd64")) { "x86.x64" } else { "ARM64" }
$components = [Collections.Generic.List[string]]@(
  "Microsoft.VisualStudio.Component.Windows1?SDK.*"
  "Microsoft.VisualStudio.Component.VC.*.$x"
)
if ($llvmPrefix.Equals(":bundled")) {
  $components.Add("Microsoft.VisualStudio.Component.VC.Llvm.Clang")
}
if ($cmakePrefix.Equals(":bundled")) {
  $components.Add("Microsoft.VisualStudio.Component.VC.CMake.Project")
}
$opts.FileName = $vswhere
$opts.Arguments = "-property installationPath -version `"[17.4,)`" -utf8 -latest -requires " +
"$components"
$proc = [Diagnostics.Process]::Start($opts)
$vsPrefix = $proc.StandardOutput.ReadToEnd().Trim()
$proc.WaitForExit()
if ($proc.ExitCode -ne 0 -or [string]::IsNullOrEmpty($vsPrefix)) {
  exitWithErr("No suitable Visual Studio installation found. One must have these components: ",
    $components)
}
if ($llvmPrefix.Equals(":bundled")) {
  $dir = if ($arch.Equals("amd64")) { "x64" } else { "ARM64" }
  $llvmPrefix = [IO.Path]::Combine($vsPrefix, "VC", "Tools", "Llvm", $dir)
}
if ($cmakePrefix.Equals(":bundled")) {
  $cmakePrefix = [IO.Path]::Combine($vsPrefix, "Common7", "IDE",
    "CommonExtensions", "Microsoft", "CMake", "CMake")
}

$path = [Environment]::GetEnvironmentVariable("PATH")

# Set SDK environment variables by stealing them from VsDevCmd & filter to not pollute
$sdkEnvVars = @(
  "INCLUDE",
  "LIB"
)
$opts.FileName = [Environment]::GetEnvironmentVariable("ComSpec")
$vsDevCmd = [IO.Path]::Combine($vsPrefix, "Common7", "Tools", "VsDevCmd.bat")

## Save some time on iterating, but add back PATH so VsDevCmd doesn't complain missing PowerShell
$opts.Environment.Clear()
$opts.Environment["PATH"] = $path

## Extract environment variables set by VsDevCmd
$opts.Arguments = "/c `"$vsDevCmd`" -arch=$arch -host_arch=$arch >NUL && set"

$proc = [Diagnostics.Process]::Start($opts)
$stdOut = $proc.StandardOutput
$regex = [Text.RegularExpressions.Regex]::new("^(.+)=(.*)$")
$lines = [Collections.Generic.List[string]]::new()
while (!$stdOut.EndOfStream) {
  $line = $stdOut.ReadLine()
  $match = $regex.Match($line)
  if (!$match.Success) {
    continue
  }
  $name = $match.Groups[1].Value
  if (!$sdkEnvVars.Contains($name)) {
    continue
  }
  $val = $match.Groups[2].Value
  [Environment]::SetEnvironmentVariable($name, $val)
  if ($ci) {

    # Since GITHUB_ENV is a file, we want to append only once so we accumulate
    $lines.Add($line)
  }
}
$proc.WaitForExit()

[Environment]::SetEnvironmentVariable("IMPORTIZER_OS", "windows")
[Environment]::SetEnvironmentVariable("IMPORTIZER_ARCH", $arch)
$llvmBin = [IO.Path]::Combine($llvmPrefix, "bin")
if (!$path.Contains($llvmBin)) {
  [Environment]::SetEnvironmentVariable("PATH", "$llvmBin;$path")
}
$cmakeBin = [IO.Path]::Combine($cmakePrefix, "bin")
if (!$path.Contains($cmakeBin)) {
  [Environment]::SetEnvironmentVariable("PATH", "$cmakeBin;$path")
}
if ($ci) {
  $ghPath = [Environment]::GetEnvironmentVariable("GITHUB_PATH")
  [IO.File]::AppendAllText($ghPath, [string[]]$cmakeBin, $utf8NoBom)
  [IO.File]::AppendAllText($ghPath, [string[]]$llvmBin, $utf8NoBom)
  $lines.Add("IMPORTIZER_OS=windows")
  $lines.Add("IMPORTIZER_ARCH=$arch")
  [IO.File]::AppendAllLines([Environment]::GetEnvironmentVariable("GITHUB_ENV"), $lines, $utf8NoBom)
}

