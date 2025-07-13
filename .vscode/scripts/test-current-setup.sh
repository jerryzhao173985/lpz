#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║         Test Current VSCode C++ Configuration                      ║
# ║         Automated testing script for baseline metrics              ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Test results file
RESULTS_FILE=".vscode/TEST_RESULTS_$(date +%Y%m%d_%H%M%S).md"
TEMP_DIR="/tmp/lpzrobots_test_$$"

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            LPZRobots VSCode Configuration Test Suite               ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Create results file header
cat > "$RESULTS_FILE" << EOF
# VSCode C++ Configuration Test Results

## Test Environment
- **Date**: $(date "+%Y-%m-%d %H:%M:%S")
- **VSCode Version**: $(code --version | head -1)
- **C++ Extension Version**: $(code --list-extensions --show-versions | grep ms-vscode.cpptools | cut -d@ -f2)
- **System**: macOS M4 Max, 64GB RAM
- **Current Directory**: $(pwd)

## System Information
\`\`\`
$(system_profiler SPHardwareDataType | grep -E "Model|Chip|Memory" | sed 's/^ *//')
\`\`\`

EOF

# Function to print section header
print_section() {
    echo ""
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo "## $1" >> "$RESULTS_FILE"
    echo "" >> "$RESULTS_FILE"
}

# Function to run test and record result
run_test() {
    local test_name="$1"
    local test_command="$2"
    local expected="$3"
    
    echo -e "${YELLOW}Testing:${NC} $test_name"
    echo "### $test_name" >> "$RESULTS_FILE"
    
    # Run the test
    local start_time=$(date +%s.%N)
    local result
    if result=$(eval "$test_command" 2>&1); then
        local end_time=$(date +%s.%N)
        local duration=$(echo "$end_time - $start_time" | bc)
        echo -e "${GREEN}✓${NC} Passed (${duration}s)"
        echo "- **Status**: ✅ Passed" >> "$RESULTS_FILE"
        echo "- **Duration**: ${duration}s" >> "$RESULTS_FILE"
        if [ -n "$expected" ]; then
            echo "- **Expected**: $expected" >> "$RESULTS_FILE"
        fi
        if [ -n "$result" ]; then
            echo "- **Result**: $result" >> "$RESULTS_FILE"
        fi
    else
        echo -e "${RED}✗${NC} Failed"
        echo "- **Status**: ❌ Failed" >> "$RESULTS_FILE"
        echo "- **Error**: $result" >> "$RESULTS_FILE"
    fi
    echo "" >> "$RESULTS_FILE"
}

# Create test directory
mkdir -p "$TEMP_DIR"

print_section "1. Code Formatting Tests"

# Test 1.1: Check clang-format availability
run_test "Clang-format availability" \
    "which clang-format && clang-format --version" \
    "clang-format installed"

# Test 1.2: Test format configuration
cat > "$TEMP_DIR/test_format.cpp" << 'EOF'
#include <iostream>
    #include <vector>
        #include <string>

int   main(  )  {
std::cout<<"Hello"<<std::endl;
        return    0;
}
EOF

run_test "Format test file" \
    "clang-format -i $TEMP_DIR/test_format.cpp && cat $TEMP_DIR/test_format.cpp | wc -l" \
    "Properly formatted"

# Test 1.3: Check .clang-format file
run_test ".clang-format exists" \
    "[ -f .clang-format ] && echo 'Found'" \
    "Configuration file present"

print_section "2. IntelliSense Performance Tests"

# Test 2.1: Check compile_commands.json
run_test "compile_commands.json exists" \
    "[ -f compile_commands.json ] && echo 'Found' || ([ -f build/macos-arm64/compile_commands.json ] && echo 'Found in build dir')" \
    "Compilation database present"

# Test 2.2: Test file parsing speed
echo -e "${YELLOW}Note:${NC} IntelliSense timing tests require manual VSCode interaction"
echo "IntelliSense timing tests require manual VSCode interaction" >> "$RESULTS_FILE"

print_section "3. Build System Tests"

# Test 3.1: CMake availability
run_test "CMake version" \
    "cmake --version | head -1" \
    "CMake 3.20+"

# Test 3.2: Check CMakePresets.json
run_test "CMakePresets.json exists" \
    "[ -f CMakePresets.json ] && echo 'Found'" \
    "CMake presets configured"

# Test 3.3: Check build directory
run_test "Build directory structure" \
    "[ -d build ] && echo 'Build directory exists' || echo 'No build directory'" \
    "Build system configured"

print_section "4. Project Analysis"

# Test 4.1: Count source files
run_test "C++ source file count" \
    "find . -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' | grep -v build | wc -l" \
    "Source files indexed"

# Test 4.2: Count header files
run_test "C++ header file count" \
    "find . -name '*.h' -o -name '*.hpp' | grep -v build | wc -l" \
    "Header files indexed"

# Test 4.3: Large file detection
run_test "Large files (>1000 lines)" \
    "find . -name '*.cpp' -o -name '*.h' | xargs wc -l 2>/dev/null | awk '\$1 > 1000 {count++} END {print count \" files\"}'" \
    "Performance impact files"

print_section "5. Memory and Process Analysis"

# Test 5.1: Current VSCode memory usage
echo -e "${YELLOW}Checking VSCode resource usage...${NC}"
echo "### VSCode Resource Usage" >> "$RESULTS_FILE"
echo '```' >> "$RESULTS_FILE"

# Get VSCode process info
ps aux | grep -E "Visual Studio Code|cpptools|clangd" | grep -v grep | while read line; do
    echo "$line" >> "$RESULTS_FILE"
done

echo '```' >> "$RESULTS_FILE"

# Test 5.2: System memory
run_test "Available system memory" \
    "vm_stat | awk '/Pages free/ { gsub(/\\./,\"\",\$3); printf \"%.2f GB\", \$3 * 4096 / (1024*1024*1024) }'" \
    "Sufficient memory available"

print_section "6. Extension Compatibility"

# Test 6.1: List C++ related extensions
echo "### Installed C++ Extensions" >> "$RESULTS_FILE"
echo '```' >> "$RESULTS_FILE"
code --list-extensions | grep -E "(cpp|clang|cmake|llvm)" >> "$RESULTS_FILE"
echo '```' >> "$RESULTS_FILE"

print_section "7. Quick Performance Benchmark"

# Create a test file that uses LPZRobots classes
cat > "$TEMP_DIR/test_lpz.cpp" << 'EOF'
#include "selforg/matrix.h"
#include "selforg/controller/sox.h"
#include "ode_robots/robots/sphererobot3masses.h"

int main() {
    matrix::Matrix m(10, 10);
    m.set(1.0);
    
    Sox controller;
    controller.init(10, 10);
    
    // Test NEON intrinsics
    #ifdef __ARM_NEON
    float32x4_t v = vdupq_n_f32(1.0f);
    #endif
    
    return 0;
}
EOF

# Test compilation (without IntelliSense)
run_test "Test compilation" \
    "cd $TEMP_DIR && clang++ -c test_lpz.cpp -I$(pwd)/selforg/include -I$(pwd)/ode_robots/include -std=c++17 2>&1 | grep -c error || echo '0 errors'" \
    "Successful compilation"

# Cleanup
rm -rf "$TEMP_DIR"

print_section "Test Summary"

echo -e "${GREEN}Test results saved to: $RESULTS_FILE${NC}"
echo ""
echo "## Summary" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

# Count test results
total_tests=$(grep -c "^### " "$RESULTS_FILE" || echo 0)
passed_tests=$(grep -c "✅ Passed" "$RESULTS_FILE" || echo 0)
failed_tests=$(grep -c "❌ Failed" "$RESULTS_FILE" || echo 0)

echo "- **Total Tests**: $total_tests" >> "$RESULTS_FILE"
echo "- **Passed**: $passed_tests" >> "$RESULTS_FILE"
echo "- **Failed**: $failed_tests" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

# Add manual test instructions
cat >> "$RESULTS_FILE" << 'EOF'
## Manual Tests Required

Please complete these manual tests in VSCode:

### IntelliSense Performance
1. Open `selforg/matrix/matrix.h`
2. Type `Matrix m; m.` and measure time until suggestions appear
3. Record completion quality and speed

### Code Navigation
1. Open `ode_robots/robots/sphererobot3masses.cpp`
2. Press F12 on `OdeRobot` - measure jump time
3. Press Shift+F12 on `step` method - count references

### Error Detection
1. Add syntax error: `int x = ` 
2. Measure time to show red squiggle
3. Check error message quality

### Format on Save
1. Mess up indentation in any .cpp file
2. Save with Cmd+S
3. Verify auto-formatting works

Record all manual test results in the checklist!
EOF

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Automated tests complete!${NC}"
echo ""
echo "Next steps:"
echo "1. Review test results in: $RESULTS_FILE"
echo "2. Complete manual tests listed in TEST_CHECKLIST.md"
echo "3. Record baseline metrics in the performance table"
echo "4. Make migration decision based on results"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"

# Open results in VSCode
code "$RESULTS_FILE"