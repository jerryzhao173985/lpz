#!/bin/bash

# Modern build script for LPZRobots
# This script replaces the old Makefile-based build system with CMake

set -e  # Exit on any error

# Configuration
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"
BUILD_TYPE="Release"
BUILD_UTILS=ON
BUILD_EXAMPLES=ON
BUILD_TESTS=OFF
USE_SYSTEM_ODE=ON
ENABLE_SIMD=ON
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show help
show_help() {
    cat << EOF
LPZRobots Modern Build Script

Usage: $0 [OPTIONS]

Options:
    -h, --help              Show this help message
    -p, --prefix PATH       Installation prefix (default: $INSTALL_PREFIX)
    -b, --build-dir DIR     Build directory (default: $BUILD_DIR)
    -t, --build-type TYPE   Build type: Debug, Release, RelWithDebInfo (default: $BUILD_TYPE)
    -j, --jobs N            Number of parallel jobs (default: $PARALLEL_JOBS)
    --no-utils              Disable building utility tools (guilogger, matrixviz)
    --no-examples           Disable building example simulations
    --enable-tests          Enable building tests
    --no-system-ode         Use bundled ODE instead of system ODE
    --no-simd               Disable SIMD optimizations
    --clean                 Clean build directory before building
    --install               Install after building (may require sudo)
    --configure-only        Only configure, don't build
    --verbose               Verbose build output
    --benchmark             Run matrix performance benchmarks after build

Examples:
    $0                      # Basic build with defaults
    $0 --prefix ~/lpzrobots # Install to home directory
    $0 --clean --install    # Clean build and install
    $0 --build-type Debug   # Debug build
    $0 --no-utils --no-examples # Minimal build (libraries only)

EOF
}

# Parse command line arguments
CLEAN=false
INSTALL=false
CONFIGURE_ONLY=false
VERBOSE=false
BENCHMARK=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -t|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --no-utils)
            BUILD_UTILS=OFF
            shift
            ;;
        --no-examples)
            BUILD_EXAMPLES=OFF
            shift
            ;;
        --enable-tests)
            BUILD_TESTS=ON
            shift
            ;;
        --no-system-ode)
            USE_SYSTEM_ODE=OFF
            shift
            ;;
        --no-simd)
            ENABLE_SIMD=OFF
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --configure-only)
            CONFIGURE_ONLY=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --benchmark)
            BENCHMARK=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Print configuration
print_status "LPZRobots Build Configuration"
echo "  Build directory: $BUILD_DIR"
echo "  Install prefix: $INSTALL_PREFIX"
echo "  Build type: $BUILD_TYPE"
echo "  Parallel jobs: $PARALLEL_JOBS"
echo "  Build utilities: $BUILD_UTILS"
echo "  Build examples: $BUILD_EXAMPLES"
echo "  Build tests: $BUILD_TESTS"
echo "  Use system ODE: $USE_SYSTEM_ODE"
echo "  Enable SIMD: $ENABLE_SIMD"
echo "  Run benchmarks: $BENCHMARK"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    print_error "CMake is required but not found. Please install CMake."
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | sed 's/cmake version //')
print_status "Using CMake version: $CMAKE_VERSION"

# Check CMake version
CMAKE_MAJOR=$(echo $CMAKE_VERSION | cut -d. -f1)
CMAKE_MINOR=$(echo $CMAKE_VERSION | cut -d. -f2)
if [[ $CMAKE_MAJOR -lt 3 ]] || [[ $CMAKE_MAJOR -eq 3 && $CMAKE_MINOR -lt 16 ]]; then
    print_error "CMake 3.16 or higher is required. Found: $CMAKE_VERSION"
    exit 1
fi

# Clean build directory if requested
if $CLEAN && [[ -d "$BUILD_DIR" ]]; then
    print_status "Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
print_status "Configuring build..."

CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
    -DBUILD_UTILS="$BUILD_UTILS"
    -DBUILD_EXAMPLES="$BUILD_EXAMPLES"
    -DBUILD_TESTS="$BUILD_TESTS"
    -DUSE_SYSTEM_ODE="$USE_SYSTEM_ODE"
    -DENABLE_SIMD="$ENABLE_SIMD"
)

if $VERBOSE; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

# Run CMake configure
if ! cmake "${CMAKE_ARGS[@]}" ..; then
    print_error "CMake configuration failed"
    exit 1
fi

print_success "Configuration completed successfully"

# Exit if configure-only is requested
if $CONFIGURE_ONLY; then
    print_status "Configure-only mode. Exiting."
    exit 0
fi

# Build
print_status "Building LPZRobots..."

BUILD_ARGS=(--build . --parallel "$PARALLEL_JOBS")

if $VERBOSE; then
    BUILD_ARGS+=(--verbose)
fi

if ! cmake "${BUILD_ARGS[@]}"; then
    print_error "Build failed"
    exit 1
fi

print_success "Build completed successfully"

# Run CPU feature detection if matrix benchmark was built
if [[ -f "$BUILD_DIR/selforg/matrix/matrix_benchmark" ]]; then
    print_status "Detecting CPU features for optimal performance..."
    "$BUILD_DIR/selforg/matrix/matrix_benchmark" --cpu-info-only 2>/dev/null || true
    
    # Run benchmarks if requested
    if $BENCHMARK; then
        print_status "Running matrix performance benchmarks..."
        "$BUILD_DIR/selforg/matrix/matrix_benchmark" || print_warning "Benchmarks failed or not available"
    fi
fi

# Install if requested
if $INSTALL; then
    print_status "Installing LPZRobots to $INSTALL_PREFIX..."
    
    # Check if we need sudo
    if [[ ! -w "$INSTALL_PREFIX" ]]; then
        print_warning "Installation directory is not writable. Using sudo..."
        if ! sudo cmake --install .; then
            print_error "Installation failed"
            exit 1
        fi
    else
        if ! cmake --install .; then
            print_error "Installation failed"
            exit 1
        fi
    fi
    
    print_success "Installation completed successfully"
    
    # Print post-install information
    echo ""
    print_status "Post-installation information:"
    echo "  Libraries installed to: $INSTALL_PREFIX/lib"
    echo "  Headers installed to: $INSTALL_PREFIX/include"
    echo "  Binaries installed to: $INSTALL_PREFIX/bin"
    
    if [[ "$BUILD_EXAMPLES" == "ON" ]]; then
        echo "  Examples installed to: $INSTALL_PREFIX/share/lpzrobots"
    fi
    
    echo ""
    echo "Make sure that $INSTALL_PREFIX/bin is in your PATH"
    echo "and $INSTALL_PREFIX/lib is in your library search path."
fi

print_success "All operations completed successfully!"

# Show what was built
echo ""
print_status "Built components:"
echo "  selforg library: ✓"
echo "  ode_robots library: ✓"  
echo "  ga_tools library: ✓"

if [[ "$BUILD_UTILS" == "ON" ]]; then
    echo "  configurator library: ✓"
    echo "  guilogger (if Qt available): ✓"
    echo "  matrixviz (if Qt available): ✓"
fi

if [[ "$BUILD_EXAMPLES" == "ON" ]]; then
    echo "  Example simulations: ✓"
fi

echo ""
print_status "You can now compile your simulations using the modern CMake build system!"
print_status "See the documentation for migration guide from the old Makefile system."