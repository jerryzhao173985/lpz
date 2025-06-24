#!/bin/bash
# Master build script for lpzrobots with full automation

# Script configuration
set -e  # Exit on error
set -o pipefail  # Pipe failures are errors

# Default configuration
DEFAULT_PREFIX="$HOME/lpzrobots"
DEFAULT_TYPE="USER"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored headers
print_header() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

# Function to print status
print_status() {
    local status=$1
    local message=$2
    
    case $status in
        "SUCCESS")
            echo -e "${GREEN}✓${NC} $message"
            ;;
        "ERROR")
            echo -e "${RED}✗${NC} $message"
            ;;
        "WARNING")
            echo -e "${YELLOW}!${NC} $message"
            ;;
        *)
            echo "  $message"
            ;;
    esac
}

# Parse command line arguments
PREFIX="${1:-$DEFAULT_PREFIX}"
TYPE="${2:-$DEFAULT_TYPE}"
CPUS=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

# Display build configuration
clear
echo -e "${BLUE}"
echo "╔═══════════════════════════════════════╗"
echo "║     LPZRobots Automated Build         ║"
echo "╚═══════════════════════════════════════╝"
echo -e "${NC}"
echo ""
print_header "Build Configuration"
echo "  Installation prefix: $PREFIX"
echo "  Installation type: $TYPE"
echo "  Parallel jobs: $CPUS"
echo "  Platform: $(uname -s) $(uname -m)"
echo ""

# Create necessary directories
print_header "Creating directories"
mkdir -p build_logs
mkdir -p "$PREFIX"/{bin,lib,include,share/lpzrobots}
print_status "SUCCESS" "Directories created"

# Clean previous builds
print_header "Cleaning previous builds"
if [ -f Makefile.conf ]; then
    make clean-all > build_logs/clean.log 2>&1 || true
    print_status "SUCCESS" "Previous builds cleaned"
else
    print_status "WARNING" "No previous configuration found"
fi

# Configure the build
print_header "Configuring build system"
./createMakefile.conf.auto.sh --prefix="$PREFIX" --type="$TYPE"
if [ $? -eq 0 ]; then
    print_status "SUCCESS" "Configuration complete"
else
    print_status "ERROR" "Configuration failed"
    exit 1
fi

# Build components
print_header "Building components"
echo ""

# Function to build a component
build_component() {
    local component=$1
    local display_name=$2
    local log_file="build_logs/${component}_build.log"
    
    echo -n "  Building $display_name... "
    
    if make -j$CPUS $component > "$log_file" 2>&1; then
        # Check for warnings
        local warnings=$(grep -c "warning:" "$log_file" 2>/dev/null || echo 0)
        if [ $warnings -gt 0 ]; then
            echo -e "${GREEN}✓${NC} (with $warnings warnings)"
        else
            echo -e "${GREEN}✓${NC}"
        fi
        
        # Install component
        echo -n "  Installing $display_name... "
        if make install_$component >> "$log_file" 2>&1; then
            echo -e "${GREEN}✓${NC}"
        else
            echo -e "${RED}✗${NC}"
            print_status "ERROR" "Failed to install $component. Check $log_file"
            return 1
        fi
    else
        echo -e "${RED}✗${NC}"
        print_status "ERROR" "Failed to build $component. Check $log_file"
        
        # Show last few lines of error
        echo "  Last errors:"
        tail -10 "$log_file" | grep -E "(error:|undefined)" | head -5 | sed 's/^/    /'
        return 1
    fi
    
    return 0
}

# Build each component
components=(
    "utils|Utility Tools"
    "selforg|Self-Organization Library"
    "ode|Physics Engine (ODE)"
    "ode_robots|Robot Simulation Library"
    "ga_tools|Genetic Algorithm Tools"
)

build_failed=0
for comp_pair in "${components[@]}"; do
    IFS='|' read -r component display_name <<< "$comp_pair"
    if ! build_component "$component" "$display_name"; then
        build_failed=1
        break
    fi
    echo ""
done

if [ $build_failed -eq 1 ]; then
    print_header "Build Failed"
    print_status "ERROR" "Build process failed. Check logs in build_logs/"
    exit 1
fi

# Setup environment script
print_header "Setting up environment"
cat > "$PREFIX/bin/lpzrobots-env" << 'EOF'
#!/bin/bash
# LPZRobots environment setup

LPZROBOTS_HOME="$(cd "$(dirname "$0")/.." && pwd)"
export PATH="$LPZROBOTS_HOME/bin:$PATH"
export LD_LIBRARY_PATH="$LPZROBOTS_HOME/lib:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="$LPZROBOTS_HOME/lib:$DYLD_LIBRARY_PATH"
export LPZROBOTS_HOME

echo "LPZRobots environment configured:"
echo "  LPZROBOTS_HOME=$LPZROBOTS_HOME"
echo "  Binaries in: $LPZROBOTS_HOME/bin"
echo "  Libraries in: $LPZROBOTS_HOME/lib"
EOF

chmod +x "$PREFIX/bin/lpzrobots-env"
print_status "SUCCESS" "Environment script created"

# Test installation
print_header "Testing installation"
if [ -x "$PREFIX/bin/selforg-config" ]; then
    version=$("$PREFIX/bin/selforg-config" --type 2>/dev/null || echo "unknown")
    print_status "SUCCESS" "selforg-config working (type: $version)"
else
    print_status "ERROR" "selforg-config not found"
fi

if [ -x "$PREFIX/bin/ode_robots-config" ]; then
    print_status "SUCCESS" "ode_robots-config found"
else
    print_status "ERROR" "ode_robots-config not found"
fi

# Build test simulation
print_header "Building test simulation"
cd ode_robots/simulations/template_sphererobot
make clean > /dev/null 2>&1
if make > build_logs/test_simulation.log 2>&1; then
    if [ -x ./start ]; then
        print_status "SUCCESS" "Test simulation built successfully"
    else
        print_status "ERROR" "Test simulation binary not created"
    fi
else
    print_status "ERROR" "Test simulation build failed"
fi
cd - > /dev/null

# Generate final report
print_header "Build Summary"
total_warnings=$(find build_logs -name "*.log" -exec grep -h "warning:" {} \; 2>/dev/null | wc -l | tr -d ' ')
echo "  Total warnings: $total_warnings"
echo "  Installation directory: $PREFIX"
echo "  Log files: build_logs/"
echo ""

# Final instructions
print_header "Installation Complete!"
echo ""
echo "To use LPZRobots, run one of these commands:"
echo ""
echo "  1. For current session only:"
echo "     ${BLUE}source $PREFIX/bin/lpzrobots-env${NC}"
echo ""
echo "  2. To add permanently to your shell:"
echo "     ${BLUE}echo 'source $PREFIX/bin/lpzrobots-env' >> ~/.bashrc${NC}"
echo "     ${BLUE}echo 'source $PREFIX/bin/lpzrobots-env' >> ~/.zshrc${NC}"
echo ""
echo "Example simulation:"
echo "  ${BLUE}cd ode_robots/simulations/template_sphererobot${NC}"
echo "  ${BLUE}./start -noshadow${NC}"
echo ""
print_status "SUCCESS" "Build completed successfully!"