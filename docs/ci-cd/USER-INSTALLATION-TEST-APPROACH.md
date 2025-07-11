# User Installation Test Approach

## Overview

The Legacy Make CI test has been redesigned to validate the actual user experience of installing and using LPZRobots. This approach is superior to the previous developer-mode testing because it ensures that end users can successfully use the installed packages.

## Key Improvements

### 1. USER Mode Configuration
Components are now configured with `--type=user`, which:
- Uses installation paths (`$PREFIX/include`, `$PREFIX/lib`) 
- Mimics what users get with `make install`
- Tests the actual deployment scenario

### 2. Proper Installation Testing
The workflow now:
1. Builds all components
2. Installs them to `$PREFIX` (like `make install`)
3. Tests building a simulation using only installed files
4. Validates that users can build their own simulations

### 3. Enhanced ODE Header Handling
The `fix-ci-paths.sh` script now properly handles system ODE:
```bash
# Creates symlinks to system ODE headers
if [ -d "/usr/include/ode" ]; then
    ln -sf /usr/include/ode/*.h include/ode-dbl/
fi
```

## Test Script Design

The `test-user-simulation.sh` script simulates a real user workflow:

1. **Environment Setup**
   - Sets `PATH` to include installed binaries
   - Uses only installed config scripts

2. **Validation Steps**
   - Checks all required tools are in PATH
   - Verifies config scripts work correctly
   - Shows actual compiler/linker flags used

3. **Build Test**
   - Navigates to a simulation directory
   - Runs `make` as a user would
   - Verifies the binary is created and executable

4. **Debugging Support**
   - Shows detailed error messages on failure
   - Displays Makefile contents for analysis
   - Lists directory structure for troubleshooting

## Benefits Over DEVEL Mode Testing

1. **Real-world Validation**: Tests what users actually experience
2. **Installation Verification**: Ensures `make install` works correctly
3. **Path Independence**: No dependency on source tree paths
4. **Better Coverage**: Tests the full installation and usage flow

## Comparison

### Old Approach (DEVEL Mode)
- Tested building from source tree
- Used source paths directly
- Didn't validate installation
- Complex path management

### New Approach (USER Mode)
- Tests installed packages
- Uses installation paths
- Validates user experience
- Simpler and more robust

## Implementation Details

### Component Configuration
```bash
# Configure for user installation
./configure --prefix="$PREFIX" --type=user --system=LINUX
```

### Installation Process
```bash
# Install libraries, headers, and tools
make PREFIX="$PREFIX" install
```

### User Simulation Test
```bash
# User adds to PATH and builds
export PATH="$PREFIX/bin:$PATH"
cd my_simulation
make
```

## Future Improvements

1. **Package Management**: Consider creating .deb/.rpm packages
2. **Windows Support**: Add MSYS2/MinGW installation tests
3. **Docker Images**: Provide pre-installed containers
4. **Integration Tests**: Run actual simulations, not just build

## Conclusion

This user-centric approach better serves the LPZRobots community by ensuring that the software works as intended for end users, not just developers. It's a more mature and professional approach to CI/CD testing.