# 2.0.0 (in progress)
- Breaking: change includeGuardPat to includeGuard.
- Breaking: Add and implement the off-by-default rule:
  - Include guards are no longer handled by default with a default regex
  - Pragma once is no longer handled by default
- Add `pragmaOnce` and `SOFComments` settings.
- Split IncludePrevention test into include guard and pragma once tests.
- Remove condition generation for local include section in transitional mode. Modules build can't use macros from other file, so it's safe to assume regular build also can't.
- Update descriptions of settings.
- Change other test to mode independent test

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