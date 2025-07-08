#\!/bin/bash
# CI-specific script to fix paths and configure components for the build
# This handles the special requirements of CI environments where paths differ from local development

set -e

echo "Configuring components for CI build..."

PREFIX="${HOME}/lpzrobots"
SRCROOT="$(pwd)"

# Function to configure a component
configure_component() {
    local component=$1
    local extra_args="${2:-}"
    
    if [ -d "$component" ] && [ -x "$component/configure" ]; then
        echo "Configuring $component..."
        cd "$component"
        ./configure --prefix="$PREFIX" --srcprefix="$SRCROOT/$component" --type=DEVEL $extra_args
        cd ..
    else
        echo "Skipping $component (no configure script found)"
    fi
}

# Configure selforg first
configure_component "selforg"

# Configure bundled ODE if needed
if [ -d "opende" ]; then
    echo "Configuring opende..."
    cd opende
    
    # The opende directory structure is pre-configured
    # Just create the necessary symlinks
    if [ ! -f "ode-dbl-config" ] && [ -f "ode-config" ]; then
        ln -sf ode-config ode-dbl-config
    fi
    
    # Create include/ode-dbl symlinks if needed
    if [ ! -d "include/ode-dbl" ] && [ -d "ode/src" ]; then
        mkdir -p include/ode-dbl
        ln -sf ../../ode/src/*.h include/ode-dbl/ 2>/dev/null || true
    fi
    
    cd ..
fi

# Configure ode_robots (needs selforg configured first)
configure_component "ode_robots"

# Configure ga_tools
configure_component "ga_tools"

# Configure GUI tools if they exist
for tool in guilogger matrixviz configurator; do
    if [ -d "$tool" ]; then
        configure_component "$tool"
    fi
done

echo "Component configuration complete"

# Create ODE compatibility headers from bundled ODE
echo "Setting up ODE compatibility headers..."
cd opende
if [ \! -d include/ode-dbl ]; then
    mkdir -p include/ode-dbl
    if [ -d ode/src ]; then
        ln -sf ../ode/*.h include/ode-dbl/
    fi
fi
cd ..

# Verify critical paths exist
echo "Verifying build environment..."
errors=0

# Check selforg
if [ \! -f "selforg/selforg-config" ]; then
    echo "ERROR: selforg/selforg-config not found after configuration"
    errors=$((errors + 1))
fi

# Check ODE headers
if [ \! -d "opende/include/ode-dbl" ] && [ \! -d "include/ode-dbl" ]; then
    echo "ERROR: ODE compatibility headers not found"
    errors=$((errors + 1))
fi

if [ $errors -gt 0 ]; then
    echo "Build environment verification failed with $errors errors"
    exit 1
fi

echo "Build environment verification passed"
echo "Ready for build"
EOF < /dev/null