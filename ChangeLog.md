# 1.1.0 (in progress)

# 1.0.1
- Fix includeGuardPat not matching the entire string to qualify as a guard
- Fix CondMinimizer always skipping `#if` instead of only when its skippable
- Fix raw string handling because the first `(` is not skipped over
- Fix integer literals not being handled properly (I didn't think about them at all LOL)
- Fix CondMinimizer not handling nested `#if` (it looks for the earliest `#endif`) when skipping
- Make condition hierarchy shorter by removing conditions for all imports
- Bump libfmt to 11.1.3, PCRE2 to 10.45-RC1, Argparse to 3.2

# 1.0.0
- First release