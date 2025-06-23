#!/bin/bash
# Script to check for memory leaks on macOS ARM64 using the Leaks tool
# Alternative to valgrind which is not available on ARM64 macOS

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}=== macOS Memory Leak Detection ===${NC}"
echo "Using macOS Leaks tool (alternative to valgrind)"
echo ""

# Function to run leak check on a binary
check_leaks() {
    local binary=$1
    local name=$2
    
    if [ ! -f "$binary" ]; then
        echo -e "${YELLOW}Warning: $binary not found${NC}"
        return
    fi
    
    echo -e "${GREEN}Checking $name for leaks...${NC}"
    
    # Run the binary and get its PID
    "$binary" &
    local pid=$!
    
    # Give it time to initialize
    sleep 2
    
    # Run leaks command
    leaks --atExit -- "$pid" > "leaks_${name}.txt" 2>&1
    
    # Kill the process
    kill $pid 2>/dev/null
    
    # Check results
    if grep -q "0 leaks for 0 total leaked bytes" "leaks_${name}.txt"; then
        echo -e "${GREEN}✓ No leaks detected in $name${NC}"
    else
        echo -e "${RED}✗ Leaks detected in $name:${NC}"
        grep "leaks for" "leaks_${name}.txt"
        echo "  Full report saved to leaks_${name}.txt"
    fi
    echo ""
}

# Function to run AddressSanitizer tests
run_asan_tests() {
    echo -e "${GREEN}Running AddressSanitizer tests...${NC}"
    
    # Test matrix library
    if [ -f "$PROJECT_ROOT/selforg/matrix/test_matrix" ]; then
        echo "Testing matrix library with ASan..."
        cd "$PROJECT_ROOT/selforg/matrix"
        
        # Compile with AddressSanitizer
        clang++ -std=c++17 -fsanitize=address -fno-omit-frame-pointer -g \
                -I. -I.. -I../include -I../include/selforg \
                test_matrix.cpp matrix.cpp -o test_matrix_asan
        
        # Run with ASan options
        ASAN_OPTIONS=detect_leaks=1:check_initialization_order=1:strict_init_order=1 \
            ./test_matrix_asan > asan_test_results.txt 2>&1
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Matrix tests passed with ASan${NC}"
        else
            echo -e "${RED}✗ Matrix tests failed with ASan${NC}"
            tail -20 asan_test_results.txt
        fi
    fi
    echo ""
}

# Function to run static analysis for memory issues
run_static_analysis() {
    echo -e "${GREEN}Running static analysis for memory issues...${NC}"
    
    # Use clang static analyzer
    scan-build --use-analyzer=/usr/bin/clang \
               -enable-checker alpha.security.MallocOverflow \
               -enable-checker alpha.unix.MallocWithAnnotations \
               -enable-checker unix.Malloc \
               -enable-checker unix.MallocSizeof \
               -o scan-build-results \
               make -C "$PROJECT_ROOT/selforg" clean all 2>&1 | \
               grep -E "(warning:|error:|leak|memory)" | head -20
    
    echo ""
}

# Main execution
echo -e "${YELLOW}Note: macOS doesn't have valgrind for ARM64.${NC}"
echo "Using alternative tools:"
echo "1. macOS Leaks tool"
echo "2. AddressSanitizer"
echo "3. Clang Static Analyzer"
echo ""

# Run AddressSanitizer tests
run_asan_tests

# Check for scan-build
if command -v scan-build &> /dev/null; then
    run_static_analysis
else
    echo -e "${YELLOW}scan-build not found. Install with: brew install llvm${NC}"
fi

# Example binaries to check with leaks tool
# Uncomment and adjust paths as needed:
# check_leaks "$PROJECT_ROOT/ode_robots/simulations/template_sphererobot/start" "sphererobot_sim"
# check_leaks "$PROJECT_ROOT/selforg/matrix/test_matrix" "matrix_test"

echo -e "${GREEN}=== Memory Check Complete ===${NC}"
echo "Summary:"
echo "- Use ASAN_OPTIONS environment variable for runtime checks"
echo "- Compile with -fsanitize=address for memory error detection"
echo "- Use 'leaks --atExit -- <pid>' for runtime leak detection"
echo "- Use 'instruments -t Leaks' for GUI-based leak detection"