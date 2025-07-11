# Legacy Make CI Build Fix Summary

## Issue Summary
The Legacy Make build in CI is failing with header include errors when building selforg. The root cause is complex interaction between:
1. Symlinked headers in `include/selforg/`
2. Include paths used during compilation
3. Relative includes within headers (e.g., `#include "configurable.h"`)

## Current Status
- Latest error: `fatal error: configurable.h: No such file or directory`
- Occurs when building selforg component
- Headers are being flattened (basename only) in `include/selforg/`

## Root Cause Analysis
1. **Symlink Structure**: Headers from subdirectories (controller/, utils/, etc.) are symlinked to a flat structure in `include/selforg/`
2. **Include Paths**: Compilation uses `-Iinclude -Iinclude/selforg`
3. **Header Dependencies**: Headers include each other with quotes (e.g., `#include "configurable.h"`)
4. **Build Location**: When building in selforg directory, relative paths get confused

## Attempted Fixes
1. ✅ Created ode-dbl symlinks - Fixed ODE header issues
2. ❌ Modified PREFIX symlinks - Broke selforg build
3. ❌ Created directory-preserving symlinks - Broke relative includes
4. ❌ Flattened header symlinks - Still failing

## Recommended Solution
The build system expects a specific header layout that's created by `make create_header_links`. The issue is that this layout needs to work for:
- Building selforg itself
- Building other components that depend on selforg
- Building from simulation directories

The best approach is to:
1. Build selforg without external include paths that interfere
2. Install headers properly after build
3. Use the installed headers for dependent builds

## Quick Fix for CI
```bash
# Build selforg in isolation
cd selforg
make clean
make CPPFLAGS="-Iinclude -Iinclude/selforg -std=c++17 -pthread -DNOCONFIGURATOR -g -O"
cd ..

# Continue with other builds
make ode_robots
```

This avoids the include path conflicts that occur when building through the root Makefile.