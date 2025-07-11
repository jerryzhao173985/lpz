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

# Create include directory symlinks for cross-component dependencies
if [ ! -d "include" ]; then
    mkdir -p include
fi

# First ensure selforg has created its header links before creating the symlink
if [ -d "selforg" ] && [ ! -d "selforg/include/selforg" ]; then
    echo "Creating selforg header links first..."
    (cd selforg && make create_header_links) || echo "Failed to create header links"
fi

# Now create the symlink to the correct location
if [ -d "selforg/include/selforg" ] && [ ! -L "include/selforg" ]; then
    ln -sf ../selforg/include/selforg include/selforg
    echo "Created symlink: include/selforg -> ../selforg/include/selforg"
fi

# Create ode-dbl symlink at root include directory for ode_robots
# This is needed when ode_robots is rebuilt from simulations
if [ ! -L "include/ode-dbl" ]; then
    if [ -d "opende/include/ode-dbl" ]; then
        ln -sf ../opende/include/ode-dbl include/ode-dbl
        echo "Created symlink: include/ode-dbl -> ../opende/include/ode-dbl"
    elif [ -d "/usr/include/ode" ]; then
        # For system ODE, create symlink to system headers
        ln -sf /usr/include/ode include/ode-dbl
        echo "Created symlink: include/ode-dbl -> /usr/include/ode"
    fi
fi

# Create symlinks in installation prefix for CI builds
# This ensures simulations can find headers when they trigger rebuilds
if [ -n "$PREFIX" ]; then
    echo "Creating symlinks in installation prefix: $PREFIX"
    mkdir -p "$PREFIX/include"
    
    # First ensure selforg has created its header links
    if [ -d "$SRCROOT/selforg" ]; then
        echo "Creating selforg header links..."
        (cd "$SRCROOT/selforg" && make create_header_links) || echo "Failed to create header links"
    fi
    
    # SKIP creating selforg symlink in PREFIX for now - it breaks selforg's own build
    # The issue is that selforg's Makefile includes -I$PREFIX/include which then
    # finds the symlinked headers with broken relative includes
    echo "Skipping $PREFIX/include/selforg symlink to avoid breaking selforg build"
    
    # Also link ode-dbl headers to PREFIX
    if [ ! -L "$PREFIX/include/ode-dbl" ]; then
        if [ -d "$SRCROOT/include/ode-dbl" ]; then
            ln -sf "$SRCROOT/include/ode-dbl" "$PREFIX/include/ode-dbl"
            echo "Created symlink: $PREFIX/include/ode-dbl -> $SRCROOT/include/ode-dbl"
        elif [ -d "$SRCROOT/opende/include/ode-dbl" ]; then
            ln -sf "$SRCROOT/opende/include/ode-dbl" "$PREFIX/include/ode-dbl"
            echo "Created symlink: $PREFIX/include/ode-dbl -> $SRCROOT/opende/include/ode-dbl"
        fi
    fi
fi

# Also create symlinks in ode_robots include directory for CI builds
if [ -d "ode_robots" ]; then
    mkdir -p ode_robots/include
    if [ ! -L "ode_robots/include/selforg" ] && [ -d "selforg" ]; then
        ln -sf ../../selforg ode_robots/include/selforg
        echo "Created symlink: ode_robots/include/selforg -> ../../selforg"
    fi
    
    # CRITICAL: Also create individual header links in ode_robots/include/selforg
    # This ensures relative includes work correctly when headers are accessed via symlinks
    mkdir -p ode_robots/include/selforg/utils
    if [ -d "selforg/utils" ]; then
        for header in selforg/utils/*.h; do
            if [ -f "$header" ]; then
                ln -sf "../../../$header" "ode_robots/include/selforg/utils/$(basename $header)"
            fi
        done
        echo "Created utils header symlinks in ode_robots/include/selforg/utils/"
    fi
    
    # Also link ode-dbl headers for ode_robots
    if [ ! -L "ode_robots/include/ode-dbl" ]; then
        if [ -d "../include/ode-dbl" ]; then
            ln -sf ../../../include/ode-dbl ode_robots/include/ode-dbl
            echo "Created symlink: ode_robots/include/ode-dbl -> ../../../include/ode-dbl"
        elif [ -d "opende/include/ode-dbl" ]; then
            ln -sf ../../opende/include/ode-dbl ode_robots/include/ode-dbl
            echo "Created symlink: ode_robots/include/ode-dbl -> ../../opende/include/ode-dbl"
        fi
    fi
fi

# Configure ode_robots (needs selforg configured first)
configure_component "ode_robots"

# Generate ode_robots-config if m4 template exists
if [ -f "ode_robots/ode_robots-config.m4" ] && [ ! -f "ode_robots/ode_robots-config" ]; then
    echo "Generating ode_robots-config from m4 template..."
    cd ode_robots
    m4 -DPREFIX="$PREFIX" -DSRCPREFIX="$SRCROOT/ode_robots" -DVERSION="1.0" ode_robots-config.m4 > ode_robots-config
    chmod +x ode_robots-config
    cd ..
fi

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

# Export PATH with config scripts for Legacy Make build
export PATH="$SRCROOT/selforg:$SRCROOT/opende:$SRCROOT/ode_robots:$PATH"
echo "Updated PATH to include config scripts: $PATH"

# Verify critical paths exist
echo "Verifying build environment..."
errors=0

# Check selforg
if [ ! -f "selforg/selforg-config" ]; then
    echo "ERROR: selforg/selforg-config not found after configuration"
    errors=$((errors + 1))
else
    chmod +x selforg/selforg-config
fi

# Check ODE config
if [ ! -f "opende/ode-dbl-config" ]; then
    echo "ERROR: opende/ode-dbl-config not found after configuration"
    errors=$((errors + 1))
else
    chmod +x opende/ode-dbl-config
fi

# Check ode_robots config if it exists
if [ -f "ode_robots/ode_robots-config" ]; then
    chmod +x ode_robots/ode_robots-config
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