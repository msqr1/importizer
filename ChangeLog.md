# 2.1.0 (in progress)
- Add a HowItWorks.md
- Bump libfmt to 11.1.4

# 2.0.0
### Breaking Changes
- Renamed `includeGuardPat` to `includeGuard`.
- Changed TOML transitional mode to `[transitional]` (was `[Transitional]`) to match the command-line and project naming conventions.

### Breaking Changes: Off-by-default rule
- Include guards and `#pragma once` are no longer processed by default.
- Introduced a new `pragmaOnce` setting to control `#pragma once` handling.

### Potentially Breaking Changes
- Updated default for `moduleInterfaceExt` to `.ixx` (was `.cppm`) for MSVC compatibility.
- Removed the `logCurrentFile` setting; importizer now automatically logs the current file being processed on errors.

### New Features
- Added the `SOFComments` setting for handling start-of-file comments (e.g., license headers).
- Enhanced the Minimizer to remove empty `#el...` statements.

### Changes and Fixes
- Separated the IncludePrevention test into distinct tests for include guards and `#pragma once`.
- Eliminated condition generation for local includes in transitional mode.
- Updated documentation for the `includeGuard` setting.
- Renamed the "Other" test to "ModeIndependent".
- Classified Minimizer and umbrella header tests as ModeIndependent.
- Made CMake precompiled headers optional (opt-in via `PCH`).
- Switched the argument parser to Taywee/Args to avoid `std::any` issues on Windows Clang #13.

# 1.1.0
### New Features and Improvements
- Added a shared directive section in transitional mode to shorten the preamble.
- Renamed `CondMinimizer` to `Minimizer`, which now also removes empty `#define`/`#undef` pairs.
- Removed all `*Action` return values from functions; actions are now performed directly using lambda expressions.
- Added support for umbrella headers by converting includes to `export import` instead of plain `import`.
- Updated PCRE2 dependency to version 10.45.

# 1.0.1
### Fixes
- Fixed `includeGuardPat` to match the entire string required for a guard.
- Corrected `CondMinimizer` to skip only skippable `#if` statements rather than all of them.
- Resolved an issue with raw string handling where the first `(` was not properly skipped.
- Fixed the handling of integer literals.
- Improved `CondMinimizer` to correctly handle nested `#if` directives (it previously looked for the earliest `#endif`).
- Simplified the condition hierarchy by removing conditions for all imports.
- Updated dependencies: bumped libfmt to 11.1.3, PCRE2 to 10.45-RC1, and Argparse to 3.2.

# 1.0.0
### First Release
