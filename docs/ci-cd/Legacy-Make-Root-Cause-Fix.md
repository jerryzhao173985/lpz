# Legacy Make CI Build - Root Cause Analysis and Fix

## Problem Summary
The Legacy Make build was failing in CI with the error:
```
fatal error: configurable.h: No such file or directory
```

This occurred when building the selforg component, even though the same build worked locally.

## Root Cause
The issue was in the `.github/scripts/fix-ci-paths.sh` script. It was creating an incorrect symlink:

```bash
# INCORRECT - links to source directory
ln -sf ../selforg include/selforg
```

This created `include/selforg -> ../selforg`, pointing to the selforg source directory rather than to the properly structured header directory at `selforg/include/selforg`.

## Why This Caused Problems
1. When selforg-config adds `-I../include` to the include path, it expects to find headers at `../include/selforg/`
2. With the incorrect symlink, `../include/selforg/` pointed to the source directory, not the header directory
3. Headers like `abstractcontroller.h` include other headers with quotes: `#include "configurable.h"`
4. The compiler couldn't find `configurable.h` because the directory structure was wrong

## The Fix
Updated `fix-ci-paths.sh` to:
1. First ensure selforg has created its header links
2. Create the symlink to the correct location:

```bash
# First ensure selforg has created its header links before creating the symlink
if [ -d "selforg" ] && [ ! -d "selforg/include/selforg" ]; then
    echo "Creating selforg header links first..."
    (cd selforg && make create_header_links) || echo "Failed to create header links"
fi

# Now create the symlink to the correct location
if [ -d "selforg/include/selforg" ] && [ ! -L "include/selforg" ]; then
    ln -sf ../selforg/include/selforg include/selforg
    echo "Created symlink: include/selforg -> ../selforg/include/selforg"
fi
```

## Verification
The fix ensures that:
- `include/selforg` correctly points to `selforg/include/selforg`
- Headers are properly structured with both hierarchical and flattened symlinks
- The include path `-I../include` finds the correctly structured headers
- Relative includes within headers work properly

## Additional Notes
- The selforg Makefile already includes subdirectories in the include path: `-I. -Icontroller -Iutils -Imatrix -Istatistictools -Iwirings`
- Headers are symlinked in both hierarchical (preserving directory structure) and flattened forms for compatibility
- The CI workflow builds selforg with serial make to avoid race conditions with header symlink creation