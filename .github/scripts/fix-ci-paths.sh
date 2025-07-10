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
    
    # First, check if we need to run configure to generate ode-dbl-config
    if [ ! -f "ode-dbl-config" ]; then
        # Generate configure script if it doesn't exist
        if [ ! -f "configure" ] && [ -f "autogen.sh" ]; then
            echo "Running autogen.sh to generate configure script..."
            chmod +x autogen.sh
            ./autogen.sh || {
                echo "Warning: autogen.sh failed, trying manual approach"
                # Manual fallback
                if [ -f "configure.in" ]; then
                    autoreconf -fvi || echo "autoreconf failed"
                fi
            }
        fi
        
        if [ -f "configure" ]; then
            echo "Running opende configure..."
            ./configure --prefix="$PREFIX" --enable-double-precision || {
                echo "Warning: opende configure failed"
            }
        elif [ -f "ode-config" ]; then
            # Fallback: create symlink if ode-config exists
            ln -sf ode-config ode-dbl-config
        else
            # Last resort: generate minimal ode-dbl-config
            echo "Warning: No configure script found, generating minimal ode-dbl-config"
            if [ -f "generate-ode-dbl-config.sh" ]; then
                chmod +x generate-ode-dbl-config.sh
                ./generate-ode-dbl-config.sh
            fi
        fi
    fi
    
    # Create include/ode-dbl symlinks if needed
    if [ ! -d "include/ode-dbl" ]; then
        mkdir -p include/ode-dbl
        if [ -d "ode/src" ]; then
            # Link header files from ode/src
            for header in ode/src/*.h; do
                if [ -f "$header" ]; then
                    ln -sf "../../$header" "include/ode-dbl/$(basename $header)"
                fi
            done
        elif [ -d "include/ode" ]; then
            # Alternative: link from include/ode
            ln -sf ../ode/*.h include/ode-dbl/
        fi
    fi
    
    cd ..
fi

# Configure ode_robots (needs selforg configured first)
configure_component "ode_robots"

# Configure ga_tools
configure_component "ga_tools"

# Configure GUI tools if they exist
for tool in guilogger matrixviz; do
    if [ -d "$tool" ]; then
        configure_component "$tool"
    fi
done

# Configure configurator separately as it has different arguments
if [ -d "configurator" ] && [ -x "configurator/configure" ]; then
    echo "Configuring configurator..."
    cd configurator
    ./configure --prefix="$PREFIX" --type=DEVEL
    cd ..
fi

echo "Component configuration complete"

# ODE compatibility headers are already set up above

# Verify critical paths exist
echo "Verifying build environment..."
errors=0

# Check selforg
if [ ! -f "selforg/selforg-config" ]; then
    echo "ERROR: selforg/selforg-config not found after configuration"
    errors=$((errors + 1))
fi

# Check ODE config
if [ ! -f "opende/ode-dbl-config" ]; then
    echo "ERROR: opende/ode-dbl-config not found after configuration"
    errors=$((errors + 1))
fi

# Check ODE headers
if [ ! -d "opende/include/ode-dbl" ]; then
    echo "ERROR: ODE compatibility headers not found at opende/include/ode-dbl"
    errors=$((errors + 1))
fi

if [ $errors -gt 0 ]; then
    echo "Build environment verification failed with $errors errors"
    exit 1
fi

echo "Build environment verification passed"
echo "Ready for build"
EOF < /dev/null