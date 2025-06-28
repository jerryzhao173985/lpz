# macOS Static Library Build Fix

## Problem
On macOS, when using `-l` flags to link libraries, the linker prefers `.so` (shared) files over `.a` (static) files, even when requesting static linking. This causes build failures when both `.so` and `.a` files exist in the same directory.

## Solution
Modified the M4 templates for the config scripts to use explicit `.a` file paths instead of `-l` flags when `--static` is specified on macOS.

## Changes Made

### 1. Modified ode_robots-config.m4
- Added `USE_STATIC_LIBS=1` flag when `--static` is used
- Changed `--libs` output to use explicit paths: `$srcprefix/lib${LIBBASE}.a` instead of `-l$LIBBASE`
- Fixed paths to remove duplicate `/ode_robots` in include and library paths
- Fixed `--srcprefix` to not append `/ode_robots` 

### 2. Modified selforg-config.m4
- Added `USE_STATIC_LIBS=1` flag when `--static` is used  
- Changed `--libs` output to use explicit paths: `$srcprefix/lib${LIBBASE}.a` instead of `-l$LIBBASE`

### 3. Regenerated Config Scripts
```bash
# For ode_robots
cd ode_robots
./configure --prefix=/Users/jerry/lpzrobot_mac --srcprefix=/Users/jerry/lpzrobot_mac/ode_robots --system=MAC_ARM64 --type=DEVEL

# For selforg
cd selforg  
./configure --prefix=/tmp/lpz --srcprefix=/Users/jerry/lpzrobot_mac/selforg --system=MAC_ARM64 --type=DEVEL
```

### 4. Regenerated Simulation Makefiles
```bash
cd /Users/jerry/lpzrobot_mac
make MODULE=ode_robots confsubmodule
```

## Result
Simulations now build correctly on macOS ARM64 with static libraries:

**Before (failing):**
```bash
g++ ... -L/path/to/libs -lode_robots_opt -lselforg_opt ...
# Linker picks .so files if they exist
```

**After (working):**
```bash
g++ ... /path/to/libs/libode_robots_opt.a /path/to/libs/libselforg_opt.a ...
# Linker uses explicit .a files
```

## Testing
To test the fix:
```bash
cd ode_robots/simulations/template_sphererobot
make clean
make opt
# Should build successfully without linker errors
```

## Notes
- This fix only applies to macOS builds when using `--static` flag
- Linux builds continue to use the standard `-l` flags with `-Wl,-Bstatic`/`-Wl,-Bdynamic`
- The fix is backwards compatible and doesn't affect existing functionality