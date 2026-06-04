using namespace System

param (
  [Parameter(Position = 0)][ValidateSet("x64", "arm64")][string]$arch,
  [ValidateNotNullOrEmpty()][string]$llvmPrefix = "C:/Program Files/LLVM"
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
    $arch = "x64"
  } else {
    exitWithErr("Unsupported architecture '", $osArch, "', only x64 or arm64 is supported")
  }
}

$opts = [Diagnostics.ProcessStartInfo]::new()
$opts.RedirectStandardOutput = $true
$opts.UseShellExecute = $false
$opts.CreateNoWindow = $true

$vswhere = [IO.Path]::Combine(
  [Environment]::GetEnvironmentVariable("ProgramFiles(x86)"), "Microsoft Visual Studio",
  "Installer", "vswhere.exe")
if (![IO.File]::Exists($vswhere)) {
  exitWithErr("vswhere not found. Visual Studio does not appear to be installed.")
}
$x = if ($arch.Equals("x64")) { "x86.x64" } else { "ARM64" }
$components = [Collections.Generic.List[string]]@(
  "Microsoft.VisualStudio.Component.Windows1?SDK.*"
  "Microsoft.VisualStudio.Component.VC.*.$x"
)
if ($llvmPrefix.Equals(":bundled")) {
  $components.Add("Microsoft.VisualStudio.Component.VC.Llvm.Clang")
}

$opts.FileName = $vswhere
$opts.Arguments = "-property installationPath -version `"[17.4,)`" -utf8 -latest -requires " + "$components"
$proc = [Diagnostics.Process]::Start($opts)
$vsPrefix = $proc.StandardOutput.ReadToEnd().Trim()
$proc.WaitForExit()
if ($proc.ExitCode -ne 0 -or [string]::IsNullOrEmpty($vsPrefix)) {
  exitWithErr("No suitable Visual Studio installation found. One must have these components: ", $components)
}

if ($llvmPrefix.Equals(":bundled")) {
  $dir = if ($arch.Equals("x64")) { "x64" } else { "ARM64" }
  $llvmPrefix = [IO.Path]::Combine($vsPrefix, "VC", "Tools", "Llvm", $dir)
}
$llvmBin = [IO.Path]::Combine($llvmPrefix, "bin")

$ci = ![string]::IsNullOrEmpty([Environment]::GetEnvironmentVariable("CI"))
$utf8NoBom = [Text.UTF8Encoding]::new($false)

$path = [Environment]::GetEnvironmentVariable("PATH")
if (!$path.Contains($llvmBin)) {
  [Environment]::SetEnvironmentVariable("PATH", "$llvmBin;$path")
  if ($ci) {
    $ghPath = [Environment]::GetEnvironmentVariable("GITHUB_PATH")
    [IO.File]::AppendAllText($ghPath, [string[]]$llvmBin, $utf8NoBom)
  }
}

$ghEnv = [Environment]::GetEnvironmentVariable("GITHUB_ENV")
$sdkVars = @(
  "INCLUDE",
  "LIB"
)
$opts.FileName = [Environment]::GetEnvironmentVariable("ComSpec")
$vsDevCmd = [IO.Path]::Combine($vsPrefix, "Common7", "Tools", "VsDevCmd.bat")

# Save some time on iterating, but add back PATH so VsDevCmd doesn't complain missing PowerShell
$opts.Environment.Clear()
$opts.Environment["PATH"] = $path

# Extract environment variables set by VsDevCmd
$opts.Arguments = "/c `"$vsDevCmd`" -arch=$arch -host_arch=$arch >NUL && set"

$proc = [Diagnostics.Process]::Start($opts)
$stdOut = $proc.StandardOutput
$regex = [Text.RegularExpressions.Regex]::new("^(.+)=(.*)$")
while (!$stdOut.EndOfStream) {
  $line = $stdOut.ReadLine()
  $match = $regex.Match($line)
  if (!$match.Success) {
    continue
  }
  $name = $match.Groups[1].Value
  if (!$sdkVars.Contains($name)) {
    continue
  }
  $val = $match.Groups[2].Value
  [Environment]::SetEnvironmentVariable($name, $val)
}
$proc.WaitForExit()

