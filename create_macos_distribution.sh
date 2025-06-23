#!/bin/bash
# LPZRobots macOS ARM64 Binary Distribution Script
# Creates a binary distribution package for macOS ARM64

set -e

# Configuration
VERSION="0.8.0"
ARCH="arm64"
DIST_NAME="lpzrobots-macos-${ARCH}-${VERSION}"
DIST_DIR="dist/${DIST_NAME}"

echo "Creating LPZRobots macOS ARM64 distribution..."
echo "Version: ${VERSION}"
echo "Architecture: ${ARCH}"

# Clean previous distribution
rm -rf dist
mkdir -p "${DIST_DIR}"

# Create directory structure
mkdir -p "${DIST_DIR}/lib"
mkdir -p "${DIST_DIR}/bin"
mkdir -p "${DIST_DIR}/include"
mkdir -p "${DIST_DIR}/share/lpzrobots"
mkdir -p "${DIST_DIR}/examples"

# Copy libraries
echo "Copying libraries..."
# selforg libraries
cp -v selforg/libselforg*.{a,so,dylib} "${DIST_DIR}/lib/" 2>/dev/null || true
# ode_robots libraries  
cp -v ode_robots/libode_robots*.{a,so} "${DIST_DIR}/lib/" 2>/dev/null || true
# configurator library
cp -v configurator/libconfigurator*.dylib "${DIST_DIR}/lib/" 2>/dev/null || true

# Copy binaries and scripts
echo "Copying binaries and scripts..."
cp -v selforg/selforg-config "${DIST_DIR}/bin/"
cp -v ode_robots/ode_robots-config "${DIST_DIR}/bin/"
cp -v configurator/configurator-config "${DIST_DIR}/bin/" 2>/dev/null || true

# Copy utilities
cp -v ode_robots/utils/*.pl "${DIST_DIR}/bin/" 2>/dev/null || true
cp -v ode_robots/utils/*.sh "${DIST_DIR}/bin/" 2>/dev/null || true

# GUI tools
if [ -f "guilogger/guilogger" ]; then
    cp -v guilogger/guilogger "${DIST_DIR}/bin/"
fi
if [ -f "matrixviz/matrixviz" ]; then
    cp -v matrixviz/matrixviz "${DIST_DIR}/bin/"
fi

# Copy headers
echo "Copying headers..."
cp -r selforg/include/* "${DIST_DIR}/include/" 2>/dev/null || true
cp -r ode_robots/include/* "${DIST_DIR}/include/" 2>/dev/null || true
cp -r configurator/include/* "${DIST_DIR}/include/" 2>/dev/null || true

# Copy data files
echo "Copying data files..."
cp -r ode_robots/osg/data "${DIST_DIR}/share/lpzrobots/" 2>/dev/null || true

# Copy example simulations
echo "Copying example simulations..."
# Select a few representative simulations
for sim in template_sphererobot template_schlange template_hexapod; do
    if [ -d "ode_robots/simulations/${sim}" ]; then
        cp -r "ode_robots/simulations/${sim}" "${DIST_DIR}/examples/"
    fi
done

# Create README
cat > "${DIST_DIR}/README.md" << EOF
# LPZRobots macOS ARM64 Distribution

Version: ${VERSION}
Architecture: ${ARCH}
Build Date: $(date)

## Installation

1. Extract this archive to your desired location
2. Add the bin directory to your PATH:
   export PATH=\$PATH:$(pwd)/${DIST_DIR}/bin

3. Set library paths:
   export DYLD_LIBRARY_PATH=\$DYLD_LIBRARY_PATH:$(pwd)/${DIST_DIR}/lib

4. For development, you may want to set:
   export LPZROBOTS_HOME=$(pwd)/${DIST_DIR}

## Usage

### Creating a new simulation:
\`\`\`bash
cd examples
cp -r template_sphererobot my_simulation
cd my_simulation
make
./start
\`\`\`

### Dependencies

Make sure you have the following installed via Homebrew:
- OpenSceneGraph: brew install open-scene-graph
- ODE: brew install ode
- Qt5: brew install qt@5
- GSL: brew install gsl
- readline: brew install readline

## Components Included

- selforg: Self-organizing control library
- ode_robots: Robot simulation framework
- configurator: Qt5-based configuration GUI
- guilogger: Real-time parameter visualization
- matrixviz: Matrix visualization tool

## Documentation

See https://github.com/georgmartius/lpzrobots for full documentation.

EOF

# Create installation script
cat > "${DIST_DIR}/install.sh" << 'EOF'
#!/bin/bash
# LPZRobots Installation Script

INSTALL_PREFIX="${1:-/usr/local}"

echo "Installing LPZRobots to ${INSTALL_PREFIX}..."

# Check permissions
if [ ! -w "${INSTALL_PREFIX}" ]; then
    echo "Error: No write permission to ${INSTALL_PREFIX}"
    echo "Try: sudo $0 ${INSTALL_PREFIX}"
    exit 1
fi

# Install files
cp -r lib/* "${INSTALL_PREFIX}/lib/"
cp -r bin/* "${INSTALL_PREFIX}/bin/"
cp -r include/* "${INSTALL_PREFIX}/include/"
mkdir -p "${INSTALL_PREFIX}/share/lpzrobots"
cp -r share/lpzrobots/* "${INSTALL_PREFIX}/share/lpzrobots/"

echo "Installation complete!"
echo "Add ${INSTALL_PREFIX}/bin to your PATH if not already included."
EOF

chmod +x "${DIST_DIR}/install.sh"

# Fix library paths for distribution
echo "Fixing library paths..."
cd "${DIST_DIR}/lib"
for lib in *.dylib; do
    if [ -f "$lib" ]; then
        # Update install names
        install_name_tool -id "@rpath/$lib" "$lib" 2>/dev/null || true
        # Update dependencies
        for dep in libselforg libode_robots libconfigurator; do
            install_name_tool -change "../selforg/${dep}.so" "@rpath/${dep}.dylib" "$lib" 2>/dev/null || true
            install_name_tool -change "../ode_robots/${dep}.so" "@rpath/${dep}.dylib" "$lib" 2>/dev/null || true
            install_name_tool -change "../configurator/${dep}.dylib" "@rpath/${dep}.dylib" "$lib" 2>/dev/null || true
        done
    fi
done
cd - > /dev/null

# Create tarball
echo "Creating distribution archive..."
cd dist
tar -czf "${DIST_NAME}.tar.gz" "${DIST_NAME}"
echo "Distribution created: dist/${DIST_NAME}.tar.gz"

# Create checksum
shasum -a 256 "${DIST_NAME}.tar.gz" > "${DIST_NAME}.tar.gz.sha256"
echo "Checksum created: dist/${DIST_NAME}.tar.gz.sha256"

echo "Done!"