# CI/CD Troubleshooting Guide for LPZRobots

## Quick Diagnosis Checklist

Before diving into specific issues, check these common problems:

1. ✓ Are all dependencies installed?
2. ✓ Is the build directory clean?
3. ✓ Are config scripts in DEVEL mode for CI?
4. ✓ Do header symlinks exist?
5. ✓ Is PATH set correctly?

## Common CI/CD Issues and Solutions

### 1. Config Script Path Issues

**Symptoms:**
```
fatal error: ode_robots/odehandle.h: No such file or directory
```

**Diagnosis:**
```bash
# Check config script mode
./selforg/selforg-config --type
# Should output "DEVEL" for CI builds
```

**Solution:**
```bash
# Regenerate in DEVEL mode
cd selforg
m4 -DPREFIX="$HOME/lpzrobots" -DSRCPREFIX="$(pwd)" \
   -DDEVEL -DLINUX -DVERSION="1.0" \
   selforg-config.m4 > selforg-config
chmod +x selforg-config
```

### 2. Missing Header Symlinks

**Symptoms:**
```
Cannot find include file: <selforg/matrix.h>
```

**Diagnosis:**
```bash
# Check if symlinks exist
ls -la selforg/include/selforg/
ls -la ode_robots/include/ode_robots/
```

**Solution:**
```bash
# Create header symlinks
cd selforg && make create_header_links
cd ../ode_robots && make create_header_links
```

### 3. Library Linking Failures

**Symptoms:**
```
undefined reference to `matrix::Matrix::Matrix()'
```

**Diagnosis:**
```bash
# Check library paths
./selforg-config --libs
# Check if libraries exist
ls -la selforg/*.a
```

**Solution:**
```bash
# Ensure libraries are built
cd selforg && make clean && make
# Verify config script outputs correct paths
./selforg-config --libfile
```

### 4. macOS-Specific Issues

#### AGL Framework Error

**Symptoms:**
```
ld: framework not found AGL
```

**Solution:**
```bash
# Remove AGL references from Qt project files
sed -i '' 's/-framework AGL//g' guilogger/src/src.pro
# Or add to .pro file:
CONFIG -= opengl
```

#### Qt6 Path Issues

**Symptoms:**
```
Project ERROR: Unknown module(s) in QT: opengl
```

**Solution:**
```bash
# Set Qt6 paths
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6:$CMAKE_PREFIX_PATH"
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
```

### 5. Simulation Build Failures

**Symptoms:**
```
make: *** No rule to make target 'main.o'
```

**Diagnosis:**
```bash
# Check Makefile generation
ls -la Makefile
# Check for M4 template
ls -la ../../Makefile.4sim.m4
```

**Solution:**
```bash
# Regenerate Makefile from template
m4 -DSRCPREFIX="$HOME/src/lpzrobots" \
   ../../Makefile.4sim.m4 > Makefile
```

### 6. CI-Specific Path Issues

**Problem:** CI uses different paths than local development

**Solution in CI workflow:**
```yaml
# Force source paths in CI
PROJECT_ROOT=$(pwd)
export PATH="$PROJECT_ROOT/selforg:$PROJECT_ROOT/ode_robots:$PATH"

# Create wrapper scripts
echo '#!/bin/bash' > selforg-config
echo "exec $PROJECT_ROOT/selforg/selforg-config --srcprefix=\"$PROJECT_ROOT/selforg\" \"\$@\"" >> selforg-config
chmod +x selforg-config
```

### 7. Virtual Display Issues (Linux CI)

**Symptoms:**
```
Cannot connect to X server
```

**Solution:**
```bash
# Set up virtual display
export DISPLAY=:99
Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &
sleep 3
```

## Debugging Commands

### Check Build Environment
```bash
# Show all relevant paths
echo "PATH: $PATH"
echo "CMAKE_PREFIX_PATH: $CMAKE_PREFIX_PATH"
echo "PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"

# Check installed packages (Ubuntu)
dpkg -l | grep -E "qt6|gsl|openscenegraph|ode"

# Check installed packages (macOS)
brew list | grep -E "qt|gsl|open-scene-graph|ode"
```

### Verify Config Scripts
```bash
# Test all config scripts
for config in selforg-config ode_robots-config ode-dbl-config; do
    echo "=== $config ==="
    ./$config --version
    ./$config --type
    ./$config --cflags
    ./$config --libs
    ./$config --libfile
done
```

### Check Library Dependencies
```bash
# Linux
ldd ./start | grep -E "selforg|ode_robots"

# macOS
otool -L ./start | grep -E "selforg|ode_robots"
```

### Verbose Build Output
```bash
# CMake build
cmake --build build --verbose

# Make build
make VERBOSE=1

# Show actual compile commands
make -n main.o
```

## CI Workflow Debugging

### Enable Debug Mode
```yaml
- name: Debug step
  run: |
    set -x  # Enable command echo
    # Your commands here
```

### Check Artifacts
```yaml
- name: Upload debug artifacts
  if: always()
  uses: actions/upload-artifact@v4
  with:
    name: debug-logs
    path: |
      build.log
      config.log
      **/CMakeCache.txt
```

### Common CI Environment Variables
```bash
# GitHub Actions
echo "GITHUB_WORKSPACE: $GITHUB_WORKSPACE"
echo "RUNNER_OS: $RUNNER_OS"
echo "RUNNER_ARCH: $RUNNER_ARCH"
```

## Platform-Specific Tips

### Ubuntu CI
- Use `ubuntu-24.04` for latest dependencies
- Install `xvfb` for headless testing
- Use `apt-get` with `-y` flag

### macOS CI
- Use `macos-15` for ARM64 support
- Handle both `/opt/homebrew` (ARM64) and `/usr/local` (x64)
- Check for Xcode command line tools

## Best Practices

1. **Always Clean Before Build**
   ```bash
   make clean || true
   rm -rf build/
   ```

2. **Use Explicit Paths**
   ```bash
   # Good
   $PROJECT_ROOT/selforg/selforg-config
   # Bad
   selforg-config  # Might find wrong version
   ```

3. **Check Return Codes**
   ```bash
   make || { echo "Build failed"; exit 1; }
   ```

4. **Log Verbose Output**
   ```bash
   make VERBOSE=1 2>&1 | tee build.log
   ```

5. **Test Incrementally**
   - Build one component at a time
   - Test config scripts after generation
   - Verify paths before building

## Getting Help

If you're still stuck:

1. Check the [CI logs](https://github.com/jerryzhao173985/lpz/actions)
2. Look for similar issues in [GitHub Issues](https://github.com/jerryzhao173985/lpz/issues)
3. Enable verbose output and check error messages
4. Compare with successful local builds
5. Ask in discussions with:
   - Full error message
   - Platform and environment details
   - Steps to reproduce