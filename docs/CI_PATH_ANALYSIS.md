# CI Path Issues Analysis and Solutions

## Executive Summary

The Legacy Make build system in LPZRobots has a sophisticated approach to handling paths that differentiates between:
1. **DEVEL mode**: For developers working from source
2. **USER mode**: For users with installed packages

In CI environments, we need to use DEVEL mode without full installation, which requires careful path management.

## Current CI Approach

### What the CI Does

1. **Uses `fix-ci-paths.sh`** to configure components:
   ```bash
   ./configure --prefix="$PREFIX" --srcprefix="$SRCROOT/$component" --type=DEVEL
   ```

2. **Creates strategic symlinks** to mimic installed structure:
   ```bash
   ln -sf ../selforg/include/selforg include/selforg
   ln -sf ../opende/include/ode-dbl include/ode-dbl
   ```

3. **Sets PATH** to find config scripts:
   ```bash
   export PATH="$PROJECT_ROOT/selforg:$PROJECT_ROOT/opende:$PROJECT_ROOT/ode_robots:$PATH"
   ```

4. **Uses environment variable** (currently commented out):
   ```bash
   export ODE_ROBOTS_CONFIG_OPTS="--srcprefix=$PROJECT_ROOT/ode_robots"
   ```

## The Problem

When simulations build in CI:
1. They look for config scripts via relative paths first
2. These config scripts are in DEVEL mode
3. But the paths they return might not resolve correctly
4. Simulations may trigger rebuilds of dependencies

## How It Should Work

### Option 1: Full Installation (Production Approach)
```bash
make all              # Builds and installs everything
cd simulations/...
make                  # Uses installed libraries/headers
```

### Option 2: Development Mode (Current CI Approach)
```bash
# Configure with source paths
./configure --type=DEVEL --srcprefix=$(pwd)
# Build without installing
make lib
# Simulations use relative paths to find components
```

## Recommended Solution

### 1. Ensure Proper DEVEL Configuration

The current approach is correct but needs refinement:

```bash
# In fix-ci-paths.sh
configure_component() {
    local component=$1
    cd "$component"
    ./configure \
        --prefix="$PREFIX" \
        --srcprefix="$SRCROOT/$component" \
        --type=DEVEL
    cd ..
}
```

### 2. Fix Include Path Structure

The key issue is that DEVEL mode expects headers in specific locations:

```bash
# selforg expects headers in include/selforg/
cd selforg && make create_header_links

# ode_robots expects to find selforg headers
mkdir -p ode_robots/include
ln -sf ../../selforg/include/selforg ode_robots/include/selforg
```

### 3. Handle Simulation Builds Correctly

For simulations, the current approach needs adjustment:

```bash
cd ode_robots/simulations/template_sphererobot

# Option A: Use environment to override paths (not currently working)
export SELFORG_CONFIG_OPTS="--srcprefix=$PROJECT_ROOT/selforg"
export ODE_ROBOTS_CONFIG_OPTS="--srcprefix=$PROJECT_ROOT/ode_robots"

# Option B: Ensure config scripts return correct paths
# This is what currently happens - config scripts are found via relative paths
```

## Why ODE_ROBOTS_CONFIG_OPTS Doesn't Work

Looking at the code:
1. The environment variable is set in CI
2. But `ode_robots-config` doesn't check for this variable
3. The config scripts use command-line arguments, not environment variables

## Correct Approach for CI

### 1. Don't Use Environment Variables
The config scripts don't support them. Instead, ensure the config scripts are properly configured during the configure step.

### 2. Use the Cascading Search
Simulations already look for config scripts in relative paths:
```makefile
SELFORGCFG := $(shell if [ -x ../../../selforg/selforg-config ]; then \
    echo "../../../selforg/selforg-config"; \
else \
    echo "selforg-config"; \
fi)
```

### 3. Ensure Headers Are Findable
The real issue is header discovery. The solution is the symlink approach already in use.

## Recommendations

1. **Remove the non-functional environment variable**:
   ```bash
   # Remove this line - it doesn't do anything
   export ODE_ROBOTS_CONFIG_OPTS="--srcprefix=$PROJECT_ROOT/ode_robots"
   ```

2. **Ensure all components are properly configured**:
   ```bash
   # This is already done correctly in fix-ci-paths.sh
   ./configure --type=DEVEL --srcprefix="$SRCROOT/$component"
   ```

3. **Focus on header symlinks**:
   The current symlink creation is the correct approach. Ensure:
   - `include/selforg` exists before ode_robots builds
   - `include/ode-dbl` exists for ODE headers
   - Component-specific include directories have proper symlinks

4. **Consider adding install step**:
   For more robust CI, consider actually installing to a temporary prefix:
   ```bash
   make all PREFIX=$HOME/ci-install
   # Then simulations will use installed versions
   ```

## Conclusion

The current CI approach is mostly correct. The main issues are:
1. The `ODE_ROBOTS_CONFIG_OPTS` environment variable doesn't work
2. Header discovery via symlinks is the correct solution
3. The DEVEL mode configuration is appropriate for CI

The Legacy Make system is designed for this use case - building from source without installation. The key is ensuring the symlink structure matches what the build system expects.