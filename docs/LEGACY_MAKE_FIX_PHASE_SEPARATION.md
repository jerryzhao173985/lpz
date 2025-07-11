# Legacy Make CI Fix: Phase Separation Strategy

## Date: 2025-01-11

## The Deep Issue

The Legacy Make system has a fundamental assumption that was being violated in our CI:
- **DEVEL mode**: Build from source tree, use relative paths
- **USER mode**: Use installed components, use PREFIX paths

Our CI was mixing these modes, causing path resolution failures.

## Root Cause Analysis

### 1. Config Script Preference
The ode_robots configure script explicitly prefers local (source tree) configs:
```bash
# From ode_robots/configure
if [ -x "$DIRNAME/../selforg/selforg-config" ]; then
    echo "Using local selforg-config at $DIRNAME/../selforg/selforg-config"
    export PATH="$DIRNAME/../selforg:$PATH"
fi
```

### 2. Path Resolution During Build
When ode_robots builds, it uses:
```makefile
SELFORGCFG=`if [ -x ../selforg/selforg-config ]; then echo "../selforg/selforg-config"; ...`
```
This means it ALWAYS prefers the source tree config over the installed one.

### 3. The Conflict
When we regenerated selforg-config for USER mode (pointing to PREFIX), but ode_robots used it during build time, it looked for headers in PREFIX/include instead of the source tree.

## The Solution: Phase Separation

### Phase 1: Build in DEVEL Mode
```bash
# Configure for DEVEL mode
m4 -DPREFIX="$(pwd)" -DSRCPREFIX="$(pwd)" -DDEVEL -DLINUX selforg-config.m4 > selforg-config
./configure --type=DEVEL

# Build with source tree paths
make
```

### Phase 2: Install with USER Mode
```bash
# Regenerate config for USER mode
m4 -DPREFIX="$PREFIX" -DSRCPREFIX="$(pwd)" -DUSER -DLINUX selforg-config.m4 > selforg-config

# Install to PREFIX
make install
```

## Key Changes Made

1. **Build all components in DEVEL mode first**
   - selforg builds with source tree paths
   - ode_robots finds selforg in ../selforg
   - No path confusion

2. **Switch to USER mode for installation**
   - Regenerate configs with PREFIX paths
   - Install headers, libs, and scripts to PREFIX
   - Configs now point to installed locations

3. **Test user workflow with installed components**
   - Simulations use installed configs from PATH
   - Headers found in PREFIX/include
   - Libraries found in PREFIX/lib

## Why This Works

1. **Build Phase**: Components find each other via relative paths in source tree
2. **Install Phase**: Components installed with correct PREFIX paths
3. **User Phase**: Simulations use only installed components

This respects the Legacy Make system's design assumptions while achieving our CI goals.

## Debugging Commands Added

```bash
# Check initial state
echo "Found selforg-config with prefix: $(selforg/selforg-config --prefix)"

# Verify after regeneration
echo "selforg-config now reports prefix: $CONFIG_PREFIX"

# Test installed configs
echo "selforg-config --cflags: $($PREFIX/bin/selforg-config --cflags)"
```

## Lessons Learned

1. **Respect build system assumptions** - Don't fight the design
2. **Separate concerns clearly** - Build vs Install vs Use
3. **Verify at each phase** - Check paths and outputs
4. **Understand preference order** - Local configs override PATH

This fix properly handles the complex interdependencies of the Legacy Make system.