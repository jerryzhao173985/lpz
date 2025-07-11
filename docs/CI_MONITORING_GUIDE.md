# CI Monitoring Guide - Legacy Make Fix

## Date: 2025-01-11

## What We Fixed

### Problem
The Legacy Make CI build was failing because:
1. Config scripts (selforg-config, ode_robots-config) had wrong prefix `/tmp/test`
2. This caused header files to not be found during compilation
3. Error: `selforg/configurable.h: No such file or directory`

### Solution Applied
1. **Force regeneration of config scripts** from M4 templates with correct PREFIX
2. **Verify config scripts** report correct prefix after generation
3. **Install headers properly** before dependent components build
4. **Remove workflow paths-ignore** so CI runs on workflow changes

## Commits Made
1. `951df31` - Ensure proper header installation in Legacy Make CI
2. `035ae0e` - Install selforg and ODE headers before building ode_robots
3. `a45f2a0` - Force regeneration of config scripts with correct PREFIX
4. `1beda09` - Allow workflow changes to trigger CI runs

## What to Monitor

### GitHub Actions Page
Go to: https://github.com/jerryzhao173985/lpz/actions

### Expected CI Behavior

1. **Quick Build Test** - Should pass (CMake build)
2. **Build on ubuntu-24.04** - Should pass
3. **Build on macos-15** - Should pass  
4. **Legacy Make build** - This is what we're fixing!

### Legacy Make Build Steps to Watch

1. **Configure components**
   - Should regenerate selforg-config with correct PREFIX
   - Should show: "selforg-config reports prefix: /home/runner/lpzrobots"
   - NOT: "prefix: /tmp/test"

2. **Build components in order**
   - selforg should build successfully
   - Should create headers in include/selforg/
   - ode_robots should find selforg headers

3. **Install components for simulation test**
   - Headers should be copied to $PREFIX/include/selforg/
   - Libraries should be copied to $PREFIX/lib/

4. **Test simulation build**
   - Should successfully build template_sphererobot
   - Should create 'start' executable

### Success Indicators
- Green checkmark on "Legacy Make build" job
- No "file not found" errors for headers
- Simulation binary created successfully

### If It Still Fails

Check for:
1. **Config script prefix** - Must match $PREFIX
2. **Header installation** - Must happen before ode_robots build
3. **Path resolution** - Config scripts must be in PATH
4. **Compilation command** - Should include correct -I flags

### Debug Commands in CI Log

Look for these outputs:
```
selforg-config reports prefix: /home/runner/lpzrobots ✓
ode_robots-config reports prefix: /home/runner/lpzrobots ✓
✓ selforg headers installed to /home/runner/lpzrobots/include/selforg
✓ Build completed successfully
✓ Simulation binary 'start' created
```

## Next Steps

1. **Monitor the CI run** triggered by commit 1beda09
2. **Check the Legacy Make build** job specifically
3. **Verify all steps pass** without header errors
4. **If successful**, the issue is resolved
5. **If failed**, examine the specific error and adjust

## Understanding the Fix

The key insight: Config scripts are build artifacts that should be regenerated
with correct paths during CI, not used as-is from version control.

This ensures the build system can find all headers and libraries in the
correct locations, whether building from source or from installed packages.