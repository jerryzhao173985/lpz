# Understanding LPZRobots Legacy Make Path System

## Overview

The LPZRobots Legacy Make build system uses a sophisticated path management approach that supports two modes:
- **DEVEL (Development)**: Uses source directories directly
- **USER (User)**: Uses installed files from PREFIX

This document explains how the path system works and how to properly use it in CI environments.

## Key Concepts

### 1. Configuration Scripts

Each component has a `-config` script generated from M4 templates:
- `selforg/selforg-config` (from `selforg-config.m4`)
- `opende/ode-dbl-config` (from `ode-dbl-config.m4`)
- `ode_robots/ode_robots-config` (from `ode_robots-config.m4`)

These scripts understand two modes via the `DEVORUSER` M4 macro:
```m4
define(`DEVORUSER', $1)  # DEVEL mode: use first argument
define(`DEVORUSER', $2)  # USER mode: use second argument
```

### 2. Path Resolution

#### DEVEL Mode (Development)
```bash
# Uses SRCPREFIX - the source directory
--cflags: -I"$srcprefix/include" -I"$srcprefix/include/ode_robots"
--libs: -L"$srcprefix/" -lode_robots
```

#### USER Mode (Installed)
```bash
# Uses PREFIX - the installation directory
--cflags: -I"$prefix/include" -I"$prefix/include/ode_robots"
--libs: -L"$prefix/lib" -lode_robots
```

### 3. Simulation Build Process

Simulations find their dependencies using a cascading search:

```makefile
# From Makefile.4sim.m4
SELFORGCFG := $(shell if [ -x ../../../selforg/selforg-config ]; then \
    echo "../../../selforg/selforg-config"; \
else \
    echo "selforg-config"; \
fi)

ODEROBOTSCFG := $(shell if [ -x ../../ode_robots-config ]; then \
    echo "../../ode_robots-config"; \
else \
    echo "ode_robots-config"; \
fi)
```

This means:
1. First look for config scripts relative to the simulation
2. Fall back to PATH (for installed versions)

## CI Environment Challenges

### Problem 1: Header Discovery

In CI, components are built but not installed. This creates issues:
- Simulations expect to find headers via `-config` scripts
- `-config` scripts in DEVEL mode point to source directories
- But headers might be scattered across the source tree

### Problem 2: Rebuild Triggers

When building a simulation:
1. Make checks if libraries exist
2. If not (or if sources changed), it triggers a rebuild
3. The rebuild uses paths from `-config` scripts
4. In CI, these paths must resolve correctly without installation

### Problem 3: Include Path Structure

The codebase expects headers in specific structures:
```
include/
├── selforg/
│   └── *.h files
├── ode_robots/
│   └── *.h files
└── ode-dbl/
    └── *.h files
```

But in source directories, headers are often directly in component roots.

## Solutions for CI

### 1. Use DEVEL Mode with Proper Configuration

Configure components with source paths:
```bash
cd selforg
./configure --prefix="$HOME/lpzrobots" --srcprefix="$(pwd)" --type=DEVEL

cd ../ode_robots
./configure --prefix="$HOME/lpzrobots" --srcprefix="$(pwd)" --type=DEVEL
```

### 2. Create Strategic Symlinks

Create symlinks to mimic installed structure:
```bash
# Root include directory
mkdir -p include
ln -sf ../selforg/include/selforg include/selforg
ln -sf ../opende/include/ode-dbl include/ode-dbl

# Component-specific includes
mkdir -p ode_robots/include
ln -sf ../../selforg ode_robots/include/selforg
ln -sf .. ode_robots/include/ode_robots
```

### 3. Set PATH for Config Scripts

Ensure config scripts are found:
```bash
export PATH="$(pwd)/selforg:$(pwd)/opende:$(pwd)/ode_robots:$PATH"
```

### 4. Handle Simulation Builds

For simulation builds in CI:
```bash
cd ode_robots/simulations/template_sphererobot

# The simulation will use relative paths to find config scripts
# These will return DEVEL mode paths pointing to source directories
make
```

## Best Practices

### 1. Always Build Components First
```bash
# Build order matters!
make selforg      # Core library
make ode         # Physics (or use system ODE)
make ode_robots  # Robot framework (depends on selforg)
```

### 2. For Production Use - Install Properly
```bash
make all         # Builds and installs everything
# This creates proper structure in PREFIX
```

### 3. For CI - Use Development Mode
```bash
# Configure each component with proper paths
.github/scripts/fix-ci-paths.sh

# Build without installation
make selforg
make ode_robots
# etc.
```

### 4. Debug Path Issues
```bash
# Check what paths are being used
./selforg/selforg-config --cflags
./ode_robots/ode_robots-config --libs

# Verify config script mode
./selforg/selforg-config --type
# Should output: DEVEL or USER
```

## Common Issues and Solutions

### Issue: "cannot find selforg/configurable.h"
**Solution**: Create include/selforg symlink or ensure selforg has run `make create_header_links`

### Issue: "ode-dbl-config not found"
**Solution**: Either generate it from opende or create symlink to system ode-config

### Issue: Simulation can't find libraries
**Solution**: Ensure component -config scripts are in PATH or accessible via relative paths

### Issue: Headers found but with wrong relative paths
**Solution**: Check if PREFIX/include has symlinks that break relative includes

## Summary

The Legacy Make system is designed for flexibility:
- **Development**: Work directly from source
- **Production**: Install to standard locations

For CI, we use DEVEL mode with careful path management to avoid needing full installation while still allowing all components to build correctly.