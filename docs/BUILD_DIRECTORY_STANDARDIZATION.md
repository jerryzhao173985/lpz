# Build Directory Standardization Plan

> **Note**: This document focuses on the CMake build system migration. The legacy Make system artifacts will gradually be phased out as components are fully migrated to CMake.

## Current State Analysis

### Scattered Build Artifacts Found:
1. **Multiple build directories**:
   - `/build/` - CMake builds
   - `/build_test/` - Test builds
   - `/build_logs/` - Build logs
   - `/dist/` - Distribution packages
   - Component-specific: `ga_tools/build*`, `selforg/build/`, `opende/build/`

2. **Legacy Make artifacts**:
   - `.o` files in `ga_tools/build_opt/`
   - `.d` dependency files scattered
   - Component-specific lib/obj directories

3. **Mixed build systems**:
   - CMake uses `build/` directories
   - Make uses in-source builds for some components

## Standardized Structure

### Recommended Directory Layout:
```
lpzrobots/
├── build/                    # All CMake build outputs (git-ignored)
│   ├── debug/
│   ├── release/
│   ├── relwithdebinfo/
│   └── ci/
├── install/                  # Installation directories (git-ignored)
│   ├── debug/
│   ├── release/
│   └── ci/
├── dist/                     # Distribution packages (git-ignored)
└── [source directories]      # Clean source tree
```

### Benefits:
1. **Single location** for all build artifacts
2. **Easy cleanup** - just `rm -rf build/`
3. **Preset alignment** - matches CMakePresets.json structure
4. **CI friendly** - predictable locations
5. **Safe** - no risk of deleting source files

## Implementation Approach

### 1. CMake-First Strategy
The build system migration focuses on CMake as the primary build system:
- All new builds should use CMake
- Legacy Make artifacts will be cleaned up naturally as components migrate
- No aggressive cleanup of source tree to avoid accidental data loss

### 2. Safe Clean Targets
CMake provides built-in clean functionality:
- `make clean` - Removes build artifacts within build directory
- `make clean-all` - Removes entire build directory (custom target)
- `make clean-check` - Shows what would be cleaned (custom target)

### 3. .gitignore Updates
The existing .gitignore already covers:
```
build/
bin/
lib/
dist/
```

No additional changes needed - these patterns catch all standard locations.

### 4. Migration Process
1. **Use out-of-source builds**: Always build with `cmake -B build/...`
2. **Clean builds**: Use `rm -rf build/` when needed (explicit and safe)
3. **Gradual transition**: Let Make artifacts naturally disappear as we migrate

## Standard Build Commands

### CMake Builds:
```bash
# Using presets (recommended):
cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix install/release

# Manual specification:
cmake -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release
cmake --install build/release --prefix install/release
```

### Safe Cleanup Commands

#### CMake build cleanup:
```bash
# From project root:
cd build/release && make clean      # Clean build artifacts
cd ../.. && rm -rf build/release    # Remove entire build directory

# Or use custom targets:
cmake --build build/release --target clean-check  # Show what would be cleaned
cmake --build build/release --target clean-all    # Remove build directory
```

#### Manual cleanup (when needed):
```bash
rm -rf build/         # Remove all CMake builds
rm -rf install/       # Remove installations
rm -rf dist/          # Remove distributions
```

**Important**: Never use `find -delete` or wildcard deletions in the source tree. Always use explicit paths when cleaning.

## Best Practices

### For Developers:
1. **Always use out-of-source builds**:
   ```bash
   cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug
   cmake --build build/debug
   ```

2. **Use CMake presets**:
   ```bash
   cmake --preset release
   cmake --build --preset release
   ```

3. **Clean builds when needed**:
   ```bash
   rm -rf build/release
   cmake --preset release
   cmake --build --preset release
   ```

### For CI/CD:
1. Start with fresh directories
2. Use predictable paths (build/, install/)
3. Clean up after builds to save space

## Verification Checklist

- [x] CMake builds use `/build/` directory
- [x] Source tree remains clean after CMake builds
- [x] .gitignore covers standard build directories
- [x] Safe clean commands documented
- [x] No dangerous cleanup scripts
- [ ] CI scripts updated to use standard paths
- [ ] Installation paths standardized

## Notes

- The legacy Make system will continue to create artifacts in source tree until fully migrated
- Focus on CMake migration rather than cleaning Make artifacts
- Always prefer explicit, safe cleanup commands over automated scripts
- Build directory can grow large - periodic manual cleanup is fine

## Transition Strategy

1. **Current**: Mixed CMake/Make builds with scattered artifacts
2. **Short term**: All new development uses CMake with standardized paths
3. **Medium term**: Migrate remaining Make components to CMake
4. **Long term**: Pure CMake build with clean source tree

The key is patience - let the migration happen naturally without risky cleanup operations.