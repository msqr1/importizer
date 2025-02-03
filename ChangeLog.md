# 1.0.1 (in progress)
- Fix includeGuardPat not matching the entire string to qualify as a guard
- Fix CondMinimizer always skipping `#if` instead of only when its skippable
- Fix raw string handling because the first `(` is not skipped over
- Fix integer literals not being handled properly (I didn't think about them at all LOL)
- Fix CondMinimizer not handling nested `#if` (it looks for the earliest `#endif`) when skipping

# 1.0.0
- First release