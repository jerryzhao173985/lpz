# Legacy Make CI Fix Summary

## Overview

This document summarizes the journey to fix the Legacy Make build in CI, documenting all issues encountered and solutions attempted.

## Issues Encountered and Fixes Applied

### 1. Initial Issue: selforg-config Not Found ✅ FIXED
**Error**: "could not find selforg-config in PATH"
**Fix**: Updated configure scripts to use relative paths (`$DIRNAME/../selforg/selforg-config`)

### 2. selforg Headers Not Found by ode_robots ✅ FIXED
**Error**: "fatal error: selforg/configurable.h: No such file or directory"
**Root Cause**: ode_robots was looking for headers in PREFIX location but they weren't installed yet
**Fix Applied**: 
- Phase 1: Build in DEVEL mode where components use source paths
- Phase 2: Switch to USER mode for installation
- This ensures headers are found during build

### 3. ode-dbl-config Not Found During Configuration ✅ FIXED
**Error**: "Error: ode-dbl-config not found" during m4 processing
**Root Cause**: ode_robots-config.m4 template requires ode-dbl-config in PATH
**Fix**: Added opende directory to PATH before processing m4 template

### 4. ODE Headers Not Found During Build ❌ CURRENT ISSUE
**Error**: "fatal error: ode-dbl/common.h: No such file or directory"
**Root Cause**: ODE compatibility layer headers aren't in the expected location
**Status**: This is the current blocker

## Build System Understanding

### Legacy Make Two-Phase Approach
1. **Phase 1 - DEVEL Mode Build**:
   - Components use source tree paths
   - selforg provides headers from `selforg/include/selforg/`
   - ode_robots finds selforg via `../selforg/selforg-config`

2. **Phase 2 - USER Mode Installation**:
   - Regenerate config scripts with installation PREFIX
   - Install headers, libraries, and config scripts
   - Test that users can build simulations

### Key Paths
- **DEVEL mode**: Uses source tree paths (`-I/path/to/source/include`)
- **USER mode**: Uses installation paths (`-I$PREFIX/include`)

## Current Status

The Legacy Make CI now:
- ✅ Successfully configures all components
- ✅ Finds selforg-config and ode-dbl-config
- ✅ Builds selforg successfully
- ✅ ode_robots finds selforg headers
- ❌ ode_robots cannot find ODE headers

## Next Steps

To fix the ODE header issue:
1. Ensure the ODE compatibility layer creates headers in `include/ode-dbl/`
2. Verify the include path `-isystem ../include/ode-dbl` is correct
3. Consider if system ODE headers need different handling in CI

## Lessons Learned

1. **Config Script Dependencies**: M4 templates execute shell commands during processing, so dependencies must be in PATH
2. **Build Order Matters**: Headers must be available before dependent components build
3. **DEVEL vs USER Modes**: Understanding the difference is crucial for fixing path issues
4. **Incremental Progress**: Each fix revealed the next issue, showing systematic progress

## Alternative Approach

Given the complexity of the Legacy Make system, consider:
1. Deprecating Legacy Make in favor of CMake (which already works in CI)
2. If Legacy Make must be maintained, simplify the build process
3. Document the exact build requirements for users