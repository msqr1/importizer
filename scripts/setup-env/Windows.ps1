param (
  [Parameter(Position = 0)][ValidateSet("x64", "arm64")][string]$arch,
  [ValidateNotNullOrEmpty()][string]$llvmPrefix = "C:/Program Files/LLVM"
)

if (!$PSBoundParameters.ContainsKey("arch")) {
  $osArch = [System.Environment]::GetEnvironmentVariable("PROCESSOR_ARCHITEW6432")
  if ([string]::IsNullOrEmpty($osArch)) {
    $osArch = [System.Environment]::GetEnvironmentVariable("PROCESSOR_ARCHITECTURE")
  }
  if ([System.Text.RegularExpressions.Regex]::IsMatch($osArch, "arm64",
      [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)) {
    $arch = "arm64"
  } elseif ([System.Text.RegularExpressions.Regex]::IsMatch($osArch, "amd64",
      [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)) {
    $arch = "x64"
  } else {
    [System.Console]::Error.WriteLine("Unsupported architecture '$osArch', only x64 or arm64 is supported")
    [System.Environment]::Exit(1)
  }
}

$opts = [System.Diagnostics.ProcessStartInfo]::new()
$opts.RedirectStandardOutput = $true
$opts.UseShellExecute = $false
$opts.CreateNoWindow = $true

$vswhere = [System.IO.Path]::Combine(
  [System.Environment]::GetEnvironmentVariable("ProgramFiles(x86)"),
  "Microsoft Visual Studio", "Installer", "vswhere.exe")
if (-not [System.IO.File]::Exists($vswhere)) {
  [System.Console]::Error.WriteLine(
    "vswhere not found. Visual Studio does not appear to be installed.")
  [System.Environment]::Exit(1)
}
$components = @(
  "Microsoft.VisualStudio.Component.Windows11SDK.26100"
)
$vswhereArgs = "-property installationPath -version `"[17.4,)`" -latest"
foreach ($c in $components) { $vswhereArgs += " -requires $c" }
$opts.FileName = $vswhere
$opts.Arguments = $vswhereArgs
$proc = [System.Diagnostics.Process]::Start($opts)
$vsPrefix = $proc.StandardOutput.ReadToEnd().Trim()
$proc.WaitForExit()
if ($proc.ExitCode -ne 0 -or [string]::IsNullOrEmpty($vsPrefix)) {
  [System.Console]::Error.WriteLine(
    "No suitable Visual Studio installation found. One must have these components:")
  foreach ($c in $components) {
    [System.Console]::Error.WriteLine("  - $c")
  }
  [System.Environment]::Exit(1)
}
[System.Console]::Out.WriteLine("$vsPrefix")

