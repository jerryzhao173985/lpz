# Deep Analysis of LPZRobots Legacy Make System

## Date: 2025-01-11

## Executive Summary

The Legacy Make build system in LPZRobots has a complex dependency chain that requires careful configuration management. The CI failures were caused by config scripts having incorrect installation prefixes, leading to header files not being found during compilation.

## System Architecture

### 1. Component Dependencies

```
selforg (core library)
    ↓
ode_robots (robot framework) 
    ↓
simulations (user code)
```

### 2. Config Script System

Each component provides a `*-config` script (similar to pkg-config):
- `selforg-config` - Provides flags for selforg library
- `ode_robots-config` - Provides flags for ode_robots
- `ode-dbl-config` - Provides flags for ODE physics engine

These scripts are generated from M4 templates:
- `selforg-config.m4` → `selforg-config`
- `ode_robots-config.m4` → `ode_robots-config`

### 3. Build Modes

**DEVEL Mode** (Development):
- Uses source tree directly
- Headers found via relative paths
- No installation required

**USER Mode** (Installation):
- Installs to PREFIX (e.g., /home/user/lpzrobots)
- Headers copied to PREFIX/include/
- Libraries copied to PREFIX/lib/
- Config scripts copied to PREFIX/bin/

## The Problem

### Root Cause
The config scripts contained hardcoded placeholder paths:
```bash
prefix="/tmp/test"  # Wrong!
```

When simulations tried to build, they would run:
```bash
selforg-config --cflags
# Returns: -I/tmp/test/include (doesn't exist!)
```

### Why It Happened

1. **Config scripts are checked into git** with placeholder values
2. **Configure scripts don't regenerate them** - they only create Makefile.conf
3. **M4 templates exist** but aren't automatically used
4. **CI was using existing scripts** instead of regenerating them

### Cascading Failures

1. selforg-config reports wrong include path
2. ode_robots fails to find selforg/configurable.h
3. Simulation build fails with missing headers

## The Solution

### 1. Force Config Script Regeneration

Always regenerate config scripts from M4 templates:
```bash
# Remove old script
rm -f selforg-config

# Generate with correct PREFIX
m4 -DPREFIX="$PREFIX" -DSRCPREFIX="$(pwd)" -DUSER -DLINUX \
   -DVERSION="$VERSION" selforg-config.m4 > selforg-config
   
# Verify it's correct
CONFIG_PREFIX=$(./selforg-config --prefix)
if [ "$CONFIG_PREFIX" != "$PREFIX" ]; then
  echo "ERROR: Wrong prefix!"
  exit 1
fi
```

### 2. Proper Installation Order

1. **Build selforg** → headers created via symlinks
2. **Install selforg** → headers copied to PREFIX/include/selforg
3. **Build ode_robots** → finds selforg headers in PREFIX
4. **Install ode_robots** → headers copied to PREFIX/include/ode_robots
5. **Build simulation** → finds all headers via config scripts

### 3. Header Path Resolution

Simulations use multiple strategies to find headers:
```makefile
# From simulation Makefile
SELFORGCFG := $(shell if [ -x ../../../selforg/selforg-config ]; then \
                echo "../../../selforg/selforg-config"; \
              else echo "selforg-config"; fi)
```

This checks:
1. Relative path (for development)
2. PATH (for installed version)

## Key Insights

### 1. M4 Template System
The M4 macro processor generates platform-specific scripts:
- `DEVEL` vs `USER` mode
- `LINUX` vs `MAC` platform
- Architecture flags (ARM64)
- Version information

### 2. Config Script Critical Functions

```bash
selforg-config --cflags    # Include paths and compile flags
selforg-config --libs      # Library paths and link flags  
selforg-config --prefix    # Installation prefix
selforg-config --type      # DEVEL or USER
```

### 3. Header Organization

**During Build**:
- Headers linked in `include/selforg/` via symlinks
- Preserves directory structure (e.g., controller/*.h)

**After Installation**:
- Headers copied to `PREFIX/include/selforg/`
- Flat structure for backward compatibility
- Hierarchical structure for new code

## CI/CD Best Practices

### 1. Always Regenerate Configs
Never trust checked-in config scripts in CI

### 2. Verify Installation
Check that headers, libraries, and scripts are properly installed

### 3. Test User Workflow
Simulate exactly what a user would do after installation

### 4. Path Independence
Ensure builds work regardless of source location

## Debugging Commands

```bash
# Check config script output
selforg-config --prefix
selforg-config --cflags

# Verify installation
ls -la $PREFIX/include/selforg/
ls -la $PREFIX/lib/libselforg*.a

# Test compilation
g++ $(selforg-config --cflags) -c test.cpp
```

## Lessons Learned

1. **Config scripts are build artifacts** - should not be in version control
2. **M4 templates are the source of truth** - always regenerate
3. **Installation testing is critical** - development builds can mask issues
4. **Path resolution is complex** - multiple fallbacks needed
5. **Verification is essential** - check outputs match expectations

## Future Improvements

1. **Remove config scripts from git** - generate during build
2. **Make configure regenerate configs** - not just Makefile.conf
3. **Add config validation** - ensure paths exist
4. **Simplify path resolution** - reduce complexity
5. **Migrate to CMake** - modern build system with better dependency management