#!/bin/bash
# Build script for LPZRobots on macOS ARM64
# This script automates the build process for all components

set -e  # Exit on error

echo "=== LPZRobots macOS ARM64 Build Script ==="
echo "This script will build all components of LPZRobots for macOS ARM64"
echo

# Set up environment
export PATH=$PATH:/Users/jerry/lpzrobots_mac/selforg:/Users/jerry/lpzrobots_mac/opende

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print status
print_status() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_progress() {
    echo -e "${YELLOW}[⚡]${NC} $1"
}

# Check prerequisites
print_progress "Checking prerequisites..."

if ! command -v brew &> /dev/null; then
    print_error "Homebrew not found. Please install Homebrew first."
    exit 1
fi

# Check for required packages
for pkg in gsl readline qt@5; do
    if brew list $pkg &> /dev/null; then
        print_status "$pkg is installed"
    else
        print_error "$pkg is not installed. Run: brew install $pkg"
        exit 1
    fi
done

# Build selforg
print_progress "Building selforg library..."
cd selforg

if [ -f libselforg.a ]; then
    print_status "selforg library already built"
else
    print_progress "Configuring selforg..."
    ./configure --system=MAC
    
    print_progress "Building selforg..."
    make clean
    make -j$(sysctl -n hw.ncpu)
    
    if [ -f libselforg.a ]; then
        print_status "selforg library built successfully"
    else
        print_error "Failed to build selforg library"
        exit 1
    fi
fi

cd ..

# Build ODE (if needed)
print_progress "Checking ODE..."
cd opende

if [ ! -f ode-dbl-config ]; then
    print_progress "Creating ode-dbl-config..."
    cat > ode-dbl-config << 'EOF'
#!/bin/sh

prefix=/Users/jerry/lpzrobots_mac/opende
exec_prefix=${prefix}
exec_prefix_set=no

usage="\
Usage: ode-dbl-config [--prefix[=DIR]] [--exec-prefix[=DIR]] [--version] [--cflags] [--libs] [--shared-libs]"

if test $# -eq 0; then
      echo "${usage}" 1>&2
      exit 1
fi

while test $# -gt 0; do
  case "$1" in
  -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) optarg= ;;
  esac

  case $1 in
    --prefix=*)
      prefix=$optarg
      if test $exec_prefix_set = no ; then
        exec_prefix=$optarg
      fi
      ;;
    --prefix)
      echo $prefix
      ;;
    --exec-prefix=*)
      exec_prefix=$optarg
      exec_prefix_set=yes
      ;;
    --exec-prefix)
      echo $exec_prefix
      ;;
    --version)
      echo 0.16.2
      ;;
    --cflags)
      echo -I${prefix}/include -I${prefix}/include/ode-dbl -DODE_DLL -DdDOUBLE
      ;;
    --libs)
      echo -L${prefix}/ode/src/.libs -lode
      ;;
    --shared-libs)
      echo -L${prefix}/ode/src/.libs -lode
      ;;
    *)
      echo "${usage}" 1>&2
      exit 1
      ;;
  esac
  shift
done
EOF
    chmod +x ode-dbl-config
    print_status "ode-dbl-config created"
fi

# Note: Full ODE build would go here if needed
# For now, we're using the stub config

cd ..

# Build ode_robots
print_progress "Building ode_robots library..."
cd ode_robots

# Remove conflicting version file
if [ -f version ]; then
    rm version
    print_status "Removed conflicting version file"
fi

if [ -f libode_robots.a ]; then
    print_status "ode_robots library already built"
else
    print_progress "Configuring ode_robots..."
    ./configure --system=MAC
    
    print_progress "Building ode_robots (this may take a while)..."
    # Add OpenGL deprecation suppression
    export CXXFLAGS="-DGL_SILENCE_DEPRECATION"
    
    # Create local ode-dbl-config wrapper
    if [ ! -f ode-dbl-config ]; then
        echo '#!/bin/sh' > ode-dbl-config
        echo 'exec /Users/jerry/lpzrobots_mac/opende/ode-dbl-config "$@"' >> ode-dbl-config
        chmod +x ode-dbl-config
    fi
    
    make clean
    make -j$(sysctl -n hw.ncpu) || true  # Continue even if some files fail
    
    if [ -f libode_robots.a ]; then
        print_status "ode_robots library built successfully"
    else
        print_error "Warning: ode_robots library build incomplete"
    fi
fi

cd ..

# Build GUI tools
print_progress "Building GUI tools..."

for tool in guilogger matrixviz; do
    if [ -d $tool ]; then
        print_progress "Building $tool..."
        cd $tool
        
        if command -v qmake &> /dev/null; then
            qmake
            make clean
            make
            print_status "$tool built"
        else
            print_error "qmake not found. Cannot build $tool"
        fi
        
        cd ..
    fi
done

echo
print_status "Build process completed!"
echo
echo "Summary:"
echo "- selforg library: $([ -f selforg/libselforg.a ] && echo "✓ Built" || echo "✗ Not built")"
echo "- ode_robots library: $([ -f ode_robots/libode_robots.a ] && echo "✓ Built" || echo "⚠ Partial")"
echo "- guilogger: $([ -f guilogger/guilogger ] && echo "✓ Built" || echo "✗ Not built")"
echo "- matrixviz: $([ -f matrixviz/matrixviz ] && echo "✓ Built" || echo "✗ Not built")"
echo
echo "Next steps:"
echo "1. If ode_robots build was incomplete, cd to ode_robots and run 'make' to continue"
echo "2. Test simulations in ode_robots/simulations/"
echo "3. Report any issues or errors"