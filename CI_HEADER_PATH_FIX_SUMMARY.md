# CI Header Path Fix Summary

## Problem
The Legacy Make CI build was failing when trying to build simulations with the error:
```
main.cpp:29:10: fatal error: ode_robots/simulation.h: No such file or directory
   29 | #include <ode_robots/simulation.h>
```

## Root Cause
The simulation code uses `#include <ode_robots/simulation.h>` which expects to find headers in an `ode_robots/` subdirectory. However:

1. The `ode_robots-config` script was outputting include paths based on `/tmp/test` prefix (from the checked-in generated file)
2. The headers weren't installed to the expected location in CI
3. The include path `-I$PREFIX/include/ode_robots` was pointing to a non-existent directory

## Solution
Added proper symlinks in the CI environment to make headers findable:

1. **In fix-ci-paths.sh**: Added creation of `ode_robots/include/ode_robots -> ..` symlink
2. **In simple-ci.yml**: Added creation of root and PREFIX level symlinks:
   - `$PROJECT_ROOT/include/ode_robots -> $PROJECT_ROOT/ode_robots`
   - `$PREFIX/include/ode_robots -> $PROJECT_ROOT/ode_robots`

## Files Modified
- `.github/scripts/fix-ci-paths.sh` - Added ode_robots header symlink creation
- `.github/workflows/simple-ci.yml` - Added symlink creation before simulation build
- `CI-STATUS-REPORT.md` - Updated to reflect the fix

## Result
The Legacy Make CI build now passes completely, including the simulation test. All 6/6 CI jobs are now passing.

## Technical Details
The fix works by creating symlinks that allow the include path resolution to work:
- When code includes `<ode_robots/simulation.h>`
- The compiler looks in `-I.../include/ode_robots`
- Which is a symlink to the actual ode_robots directory
- So it finds `.../include/ode_robots/simulation.h` -> `.../ode_robots/simulation.h`

This maintains compatibility with the existing include style without modifying source code.