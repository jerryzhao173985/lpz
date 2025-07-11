# CI/CD Troubleshooting Guide

## Common CI/CD Issues and Solutions

### 1. Config Script Issues

#### Problem: "SRCPREFIX/libselforg.a" Error
**Symptom:**
```
/usr/bin/ld: cannot find SRCPREFIX/libselforg.a
```

**Cause:** Config scripts haven't been properly generated from M4 templates.

**Solution:**
```bash
# Regenerate config scripts
cd selforg
./configure --prefix=/your/install/path --srcprefix=$(pwd) --type=USER
cd ../ode_robots
./configure --prefix=/your/install/path --srcprefix=$(pwd) --type=USER
```

#### Problem: "ode-dbl-config not found"
**Solution:**
```bash
cd opende
./generate-ode-dbl-config.sh
# Or if using system ODE:
sudo apt-get install libode-dev
```

### 2. Library Linking Issues

#### Problem: Libraries not found during simulation build
**Diagnosis:**
```bash
# Check what paths the config scripts return
./selforg/selforg-config --libfile
./ode_robots/ode_robots-config --libfile

# Verify libraries exist at those paths
ls -la $(./selforg/selforg-config --libfile)
```

**Common Fixes:**
1. Build libraries first: `make selforg ode_robots`
2. Use correct config script paths in simulations
3. For macOS: Use `--static` flag for static linking

### 3. Header Include Issues

#### Problem: "ode_robots/simulation.h not found"
**Solution:**
```bash
# Create header structure
cd ode_robots
make create_header_links
# Or manually:
mkdir -p include/ode_robots
ln -sf .. include/ode_robots
```

#### Problem: "ode/ode.h not found"
**Solution:**
```bash
# For bundled ODE
mkdir -p include/ode-dbl
ln -sf ../opende/include/ode/* include/ode-dbl/

# For system ODE
ln -sf /usr/include/ode include/ode-dbl
```

### 4. CI-Specific Issues

#### Problem: Different behavior in CI vs local
**Debugging Steps:**
1. Check CI environment variables
2. Verify paths: `echo $PATH`, `pwd`, `ls -la`
3. Use verbose make: `make VERBOSE=1`
4. Add debugging to config scripts

#### Problem: Qt6 not found on macOS CI
**Solution:** Update CMake prefix path:
```yaml
- name: Configure
  run: |
    QT_PREFIX="/opt/homebrew/opt/qt@6"  # ARM64
    cmake -DCMAKE_PREFIX_PATH="$QT_PREFIX" ...
```

### 5. Build System Selection

#### When to use CMake vs Make
- **CMake**: Preferred for CI, cross-platform builds
- **Make**: Required for simulations, legacy compatibility

#### Switching between build systems:
```bash
# CMake build
cmake --preset=ci
cmake --build build/ci

# Make build
make conf
make all
```

### 6. Simulation Testing Issues

#### Problem: "Cannot open display" error
**Solution:** Use virtual display:
```bash
export DISPLAY=:99
Xvfb :99 -screen 0 1024x768x24 &
./start -noshadow -nographics
```

#### Problem: Simulation hangs in CI
**Solution:** Use timeout and step limit:
```bash
timeout 60s ./start -noshadow -nographics -steps 100
```

### 7. Platform-Specific Issues

#### macOS ARM64 Issues
- Use `-arch arm64` flag in config scripts
- Check for universal binaries: `file ./start`
- Verify architecture: `uname -m`

#### Linux Distribution Differences
- Ubuntu 24.04: Uses Qt6 packages
- Older Ubuntu: May need Qt5 fallback
- Check package names: `apt-cache search qt6-base`

### 8. Performance Issues

#### Slow CI Builds
1. Enable ccache: Already configured in CI
2. Use parallel builds: `-j$(nproc)`
3. Cache dependencies between runs
4. Consider splitting tests into separate jobs

#### Out of Memory
- Reduce parallel jobs: `-j2` instead of `-j$(nproc)`
- Disable optimization for debug builds
- Use swap file in CI if needed

### 9. Debugging CI Failures

#### Getting More Information
1. **Enable verbose output:**
   ```yaml
   - name: Build
     run: make VERBOSE=1 2>&1 | tee build.log
   ```

2. **Upload logs as artifacts:**
   ```yaml
   - name: Upload logs
     if: failure()
     uses: actions/upload-artifact@v4
     with:
       name: build-logs
       path: |
         build.log
         config.log
   ```

3. **SSH debugging (GitHub Actions):**
   - Use `mxschmitt/action-tmate@v3` for interactive debugging

### 10. Common Error Messages

#### "multiple definition of" linker errors
- Check for duplicate symbols
- Ensure proper include guards
- Verify no accidental includes of .cpp files

#### "undefined reference to" errors
- Check library link order (dependencies last)
- Verify all required libraries are linked
- Check for missing template instantiations

#### Permission denied errors
- Ensure scripts are executable: `chmod +x script.sh`
- Check file ownership in CI
- Verify write permissions for build directory

## Quick Diagnostic Commands

```bash
# Check environment
echo "PATH: $PATH"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo "Current dir: $(pwd)"
which g++ cmake make

# Check dependencies
pkg-config --list-all | grep -E "gsl|qt|ode"
ldconfig -p | grep -E "libode|libgsl|libQt"

# Check build files
find . -name "*.a" -o -name "*.so" -o -name "*.dylib" | sort
find . -name "*-config" -executable | sort

# Test config scripts
for cfg in $(find . -name "*-config" -executable); do
    echo "=== $cfg ==="
    $cfg --version
    $cfg --cflags
    $cfg --libs
done
```

## Getting Help

1. Check CI logs carefully - the error is usually there
2. Run the same commands locally to reproduce
3. Check the [CI/CD Comprehensive Review](../CI_CD_COMPREHENSIVE_REVIEW.md)
4. Open an issue with:
   - Full error message
   - Platform and configuration
   - Steps to reproduce
   - Relevant config files