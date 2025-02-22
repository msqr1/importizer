# 2.0.0 (in progress)
- Breaking: Rename includeGuardPat to includeGuard.
- Breaking: Off-by-default rule added:
  - Include guards and #pragma once are no longer handled by default.
  - New pragmaOnce setting controls #pragma once handling.
- Breaking: Transitional mode in TOML now must be specified as `[transitional]` instead of `[Transitional]` to keep it consistent with the command line and project naming convention
- Maybe Breaking: moduleInterfaceExt default changed to ".ixx" (from ".cppm") for MSVC compatibility.
- New:
  - SOFComments setting for handling start-of-file comments (e.g., licenses).
  - Minimizer now removes empty #el... statements.
- Changes & Fixes:
  - Split IncludePrevention test into include guard and #pragma once tests.
  - Removed condition generation for local includes in transitional mode.
  - Updated descriptions for includeGuard settings.
  - Renamed test Other to ModeIndependent.
  - Minimizer and umbrella header tests are now ModeIndependent.
  - CMake precompiled headers are now optional (opt-in with PCH).
  - Switched argument parser to Taywee/Args (avoids std::any issues on Windows Clang #13).

# 1.1.0
- Add a shared directive section in transitional mode to shorten the preamble.
- CondMinimizer renamed to Minimizer that will also remove empty #define/#undef pair.
- Remove all *Action return from function. Perform the action directly and shorten code with a lambda.
- Add umbrella header support, converting includes to `export import` instead of `import`.
- Bump PCRE2 to 10.45.

# 1.0.1
- Fix includeGuardPat not matching the entire string to qualify as a guard.
- Fix CondMinimizer always skipping `#if` instead of only when its skippable.
- Fix raw string handling because the first `(` is not skipped over.
- Fix integer literals not being handled properly (I didn't think about them at all LOL).
- Fix CondMinimizer not handling nested `#if` (it looks for the earliest `#endif`) when skipping.
- Make condition hierarchy shorter by removing conditions for all imports.
- Bump libfmt to 11.1.3, PCRE2 to 10.45-RC1, Argparse to 3.2.

# 1.0.0
- First release.