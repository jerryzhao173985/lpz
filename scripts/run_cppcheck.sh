#!/bin/bash
# LPZRobots cppcheck analysis script
# Run static analysis on the entire codebase

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Running cppcheck analysis on LPZRobots...${NC}"

# Create reports directory
REPORT_DIR="$PROJECT_ROOT/cppcheck_reports"
mkdir -p "$REPORT_DIR"

# Common cppcheck options
CPPCHECK_OPTS="--enable=all \
    --std=c++17 \
    --platform=unix64 \
    --inline-suppr \
    --suppress=missingIncludeSystem \
    --suppress=unmatchedSuppression \
    --suppress=unknownMacro \
    --suppress=unusedFunction \
    -I$PROJECT_ROOT/selforg/include \
    -I$PROJECT_ROOT/ode_robots/include \
    -I/opt/homebrew/include \
    -I/usr/local/include \
    -DNDEBUG"

# Function to run cppcheck on a component
run_component_check() {
    local component=$1
    local component_path=$2
    
    echo -e "${YELLOW}Analyzing $component...${NC}"
    
    # Run cppcheck and save results
    cppcheck $CPPCHECK_OPTS \
        --xml \
        --xml-version=2 \
        "$component_path" \
        2> "$REPORT_DIR/${component}_report.xml"
    
    # Also create a text report for critical issues
    cppcheck $CPPCHECK_OPTS \
        --template='{file}:{line}: {severity}: {message} [{id}]' \
        "$component_path" \
        2>&1 | grep -E "(error:|warning:)" > "$REPORT_DIR/${component}_critical.txt"
    
    # Count issues
    local error_count=$(grep -c "error:" "$REPORT_DIR/${component}_critical.txt" || echo 0)
    local warning_count=$(grep -c "warning:" "$REPORT_DIR/${component}_critical.txt" || echo 0)
    
    echo "  Errors: $error_count"
    echo "  Warnings: $warning_count"
    
    return $error_count
}

# Run analysis on each component
total_errors=0

run_component_check "selforg" "$PROJECT_ROOT/selforg"
((total_errors+=$?))

run_component_check "ode_robots" "$PROJECT_ROOT/ode_robots"
((total_errors+=$?))

run_component_check "matrixviz" "$PROJECT_ROOT/matrixviz"
((total_errors+=$?))

run_component_check "guilogger" "$PROJECT_ROOT/guilogger"
((total_errors+=$?))

run_component_check "configurator" "$PROJECT_ROOT/configurator"
((total_errors+=$?))

# Generate summary report
echo -e "\n${GREEN}Generating summary report...${NC}"
cat > "$REPORT_DIR/summary.txt" <<EOF
LPZRobots Static Analysis Summary
Generated: $(date)

Component Analysis Results:
EOF

for component in selforg ode_robots matrixviz guilogger configurator; do
    if [ -f "$REPORT_DIR/${component}_critical.txt" ]; then
        echo -e "\n=== $component ===" >> "$REPORT_DIR/summary.txt"
        echo "Errors: $(grep -c "error:" "$REPORT_DIR/${component}_critical.txt" || echo 0)" >> "$REPORT_DIR/summary.txt"
        echo "Warnings: $(grep -c "warning:" "$REPORT_DIR/${component}_critical.txt" || echo 0)" >> "$REPORT_DIR/summary.txt"
    fi
done

# Show critical errors
if [ $total_errors -gt 0 ]; then
    echo -e "\n${RED}Critical errors found:${NC}"
    grep "error:" "$REPORT_DIR"/*_critical.txt | head -20
else
    echo -e "\n${GREEN}No critical errors found!${NC}"
fi

echo -e "\n${GREEN}Analysis complete. Reports saved in: $REPORT_DIR${NC}"
echo "View detailed reports:"
echo "  - Summary: $REPORT_DIR/summary.txt"
echo "  - Component reports: $REPORT_DIR/*_critical.txt"

exit $total_errors