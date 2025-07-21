#!/bin/bash
# Cppcheck static analysis script for LPZRobots
# This script performs comprehensive static analysis on the codebase
# using best practices for large C++ projects (2024)

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Performance timer
start_time=$(date +%s)

# Check if cppcheck is installed
if ! command -v cppcheck &> /dev/null; then
    echo -e "${RED}Error: cppcheck is not installed${NC}"
    echo "Install with: brew install cppcheck (macOS) or apt-get install cppcheck (Ubuntu)"
    exit 1
fi

# Get cppcheck version
CPPCHECK_VERSION=$(cppcheck --version | cut -d' ' -f2)
echo -e "${BLUE}Using cppcheck version: $CPPCHECK_VERSION${NC}"

# Check if version supports required features
MIN_VERSION="2.10"
if [ "$(printf '%s\n' "$MIN_VERSION" "$CPPCHECK_VERSION" | sort -V | head -n1)" != "$MIN_VERSION" ]; then
    echo -e "${YELLOW}Warning: cppcheck version $CPPCHECK_VERSION may be too old. Recommend $MIN_VERSION or newer.${NC}"
fi

# Set project root (one level up from scripts directory)
PROJECT_ROOT="$(dirname "$(dirname "$(readlink -f "$0" 2>/dev/null || realpath "$0")")")"
cd "$PROJECT_ROOT"

# Parse command line arguments
ENABLE_CTU=false
ENABLE_MISRA=false
ENABLE_CERT=false
GENERATE_HTML=false
QUICK_MODE=false
INCREMENTAL=true
THREADS=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

while [[ $# -gt 0 ]]; do
    case $1 in
        --ctu)
            ENABLE_CTU=true
            shift
            ;;
        --misra)
            ENABLE_MISRA=true
            shift
            ;;
        --cert)
            ENABLE_CERT=true
            shift
            ;;
        --html)
            GENERATE_HTML=true
            shift
            ;;
        --quick)
            QUICK_MODE=true
            shift
            ;;
        --no-cache)
            INCREMENTAL=false
            shift
            ;;
        -j|--threads)
            THREADS="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --ctu          Enable cross-translation unit analysis"
            echo "  --misra        Enable MISRA addon (requires misra.py)"
            echo "  --cert         Enable CERT addon (premium only)"
            echo "  --html         Generate HTML report"
            echo "  --quick        Quick mode (fewer checks)"
            echo "  --no-cache     Disable incremental analysis"
            echo "  -j, --threads  Number of parallel threads (default: $THREADS)"
            echo "  -h, --help     Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Create output directories
OUTPUT_DIR="build/cppcheck"
CACHE_DIR="$OUTPUT_DIR/cache"
REPORTS_DIR="$OUTPUT_DIR/reports"
mkdir -p "$OUTPUT_DIR" "$CACHE_DIR" "$REPORTS_DIR"

# Create or update suppressions file
SUPPRESSIONS_FILE="$PROJECT_ROOT/cppcheck_suppressions.txt"
if [ ! -f "$SUPPRESSIONS_FILE" ]; then
    echo -e "${YELLOW}Creating suppressions file...${NC}"
    cat > "$SUPPRESSIONS_FILE" << 'EOF'
# System header suppressions
missingIncludeSystem
unmatchedSuppression

# External library suppressions
*:*/third_party/*
*:*/external/*
*:*/build/*
*:*/.cache/*

# Known false positives in ODE
uninitvar:opende/ode/src/joints/*.cpp
nullPointer:opende/ode/src/collision_libccd.cpp

# Qt-generated code
*:*/moc_*.cpp
*:*/ui_*.h

# Suppress warnings in test code (optional)
unusedFunction:*/tests/*
EOF
fi

# Components to analyze
COMPONENTS=(
    "selforg"
    "ode_robots"
    "ga_tools"
    "opende"
)

# Build common cppcheck arguments
COMMON_ARGS=(
    --std=c++17
    --platform=unix64
    --suppressions-list="$SUPPRESSIONS_FILE"
    --inline-suppr
    -j "$THREADS"
)

# Add performance optimizations
if [ "$INCREMENTAL" = true ]; then
    COMMON_ARGS+=("--cppcheck-build-dir=$CACHE_DIR")
fi

# Configure check level
if [ "$QUICK_MODE" = true ]; then
    COMMON_ARGS+=("--enable=warning,performance,portability")
    echo -e "${YELLOW}Running in quick mode (reduced checks)${NC}"
else
    COMMON_ARGS+=("--enable=all")
    COMMON_ARGS+=("--inconclusive")
    if [ "$ENABLE_CTU" = true ]; then
        COMMON_ARGS+=("--max-ctu-depth=2")
        echo -e "${CYAN}CTU analysis enabled (slower but more thorough)${NC}"
    fi
fi

# Function to run analysis on a component
analyze_component() {
    local component=$1
    echo -e "\n${YELLOW}Analyzing $component...${NC}"
    
    # Component-specific settings
    local COMPONENT_ARGS=("${COMMON_ARGS[@]}")
    local INCLUDES=""
    local EXCLUDES=""
    
    # Add component-specific includes
    if [ -d "$component/include" ]; then
        COMPONENT_ARGS+=("-I" "$component/include")
    fi
    
    # Component-specific configurations
    case $component in
        "selforg")
            COMPONENT_ARGS+=("-I" "selforg/controller")
            COMPONENT_ARGS+=("-I" "selforg/utils")
            EXCLUDES="-i selforg/examples -i selforg/tests"
            ;;
        "ode_robots")
            COMPONENT_ARGS+=("-I" "ode_robots/osg")
            COMPONENT_ARGS+=("-I" "ode_robots/robots")
            COMPONENT_ARGS+=("-I" "selforg/include")
            COMPONENT_ARGS+=("-I" "opende/include")
            EXCLUDES="-i ode_robots/simulations -i ode_robots/examples"
            ;;
        "ga_tools")
            COMPONENT_ARGS+=("-I" "selforg/include")
            EXCLUDES="-i ga_tools/tests"
            ;;
        "opende")
            COMPONENT_ARGS+=("-I" "opende/ode/src")
            COMPONENT_ARGS+=("--suppress=uninitvar")
            EXCLUDES="-i opende/drawstuff -i opende/tests"
            ;;
    esac
    
    # Output files
    local TEXT_REPORT="$REPORTS_DIR/${component}_report.txt"
    local XML_REPORT="$REPORTS_DIR/${component}_report.xml"
    
    # Run cppcheck
    echo -e "${CYAN}Command: cppcheck ${COMPONENT_ARGS[*]} $EXCLUDES $component${NC}"
    
    if [ "$GENERATE_HTML" = true ]; then
        # Generate XML for HTML conversion
        cppcheck \
            "${COMPONENT_ARGS[@]}" \
            --xml \
            --xml-version=2 \
            --output-file="$XML_REPORT" \
            $EXCLUDES \
            "$component" 2>&1 | tee "$TEXT_REPORT"
    else
        # Text output only
        cppcheck \
            "${COMPONENT_ARGS[@]}" \
            $EXCLUDES \
            "$component" 2> "$TEXT_REPORT"
    fi
    
    # Analyze results
    local ERRORS=$(grep -c "\[error\]" "$TEXT_REPORT" 2>/dev/null || echo 0)
    local WARNINGS=$(grep -c "\[warning\]" "$TEXT_REPORT" 2>/dev/null || echo 0)
    local STYLE=$(grep -c "\[style\]" "$TEXT_REPORT" 2>/dev/null || echo 0)
    local PERFORMANCE=$(grep -c "\[performance\]" "$TEXT_REPORT" 2>/dev/null || echo 0)
    local PORTABILITY=$(grep -c "\[portability\]" "$TEXT_REPORT" 2>/dev/null || echo 0)
    local INFORMATION=$(grep -c "\[information\]" "$TEXT_REPORT" 2>/dev/null || echo 0)
    
    # Display results with color coding
    echo -e "  ${RED}Errors: $ERRORS${NC}"
    echo -e "  ${YELLOW}Warnings: $WARNINGS${NC}"
    echo -e "  ${BLUE}Style: $STYLE${NC}"
    echo -e "  ${GREEN}Performance: $PERFORMANCE${NC}"
    echo -e "  ${PURPLE}Portability: $PORTABILITY${NC}"
    echo -e "  ${CYAN}Information: $INFORMATION${NC}"
    
    # Show top issues
    if [ $((ERRORS + WARNINGS)) -gt 0 ]; then
        echo -e "\n  ${YELLOW}Top issues:${NC}"
        grep -E "\[(error|warning)\]" "$TEXT_REPORT" | head -5 | while IFS= read -r line; do
            echo "    $line"
        done
        if [ $((ERRORS + WARNINGS)) -gt 5 ]; then
            echo "    ... and $((ERRORS + WARNINGS - 5)) more"
        fi
    fi
    
    # Return status for quality gate
    echo "$component:$ERRORS:$WARNINGS:$STYLE:$PERFORMANCE:$PORTABILITY:$INFORMATION" >> "$OUTPUT_DIR/metrics.txt"
}

# Clear previous metrics
> "$OUTPUT_DIR/metrics.txt"

# Run analysis on each component
for component in "${COMPONENTS[@]}"; do
    analyze_component "$component"
done

# Run addons if requested
if [ "$ENABLE_MISRA" = true ]; then
    echo -e "\n${YELLOW}Running MISRA addon...${NC}"
    if [ -f "$(which python3)" ] && [ -f "/usr/local/share/cppcheck/addons/misra.py" ]; then
        for component in "${COMPONENTS[@]}"; do
            cppcheck --dump "$component" 2>/dev/null
            find "$component" -name "*.dump" -exec python3 /usr/local/share/cppcheck/addons/misra.py {} \; > "$REPORTS_DIR/${component}_misra.txt"
        done
    else
        echo -e "${RED}MISRA addon not found. Please ensure cppcheck addons are installed.${NC}"
    fi
fi

# Generate HTML report if requested
if [ "$GENERATE_HTML" = true ]; then
    echo -e "\n${YELLOW}Generating HTML report...${NC}"
    
    # Merge all XML reports
    XML_MERGED="$REPORTS_DIR/merged_report.xml"
    echo '<?xml version="1.0" encoding="UTF-8"?>' > "$XML_MERGED"
    echo '<results version="2">' >> "$XML_MERGED"
    echo "<cppcheck version=\"$CPPCHECK_VERSION\"/>" >> "$XML_MERGED"
    echo '<errors>' >> "$XML_MERGED"
    
    for component in "${COMPONENTS[@]}"; do
        if [ -f "$REPORTS_DIR/${component}_report.xml" ]; then
            xmllint --xpath "//error" "$REPORTS_DIR/${component}_report.xml" 2>/dev/null >> "$XML_MERGED" || true
        fi
    done
    
    echo '</errors>' >> "$XML_MERGED"
    echo '</results>' >> "$XML_MERGED"
    
    # Generate HTML
    if command -v cppcheck-htmlreport &> /dev/null; then
        HTML_DIR="$REPORTS_DIR/html"
        cppcheck-htmlreport \
            --file="$XML_MERGED" \
            --title="LPZRobots Static Analysis" \
            --report-dir="$HTML_DIR" \
            --source-dir="$PROJECT_ROOT"
        echo -e "${GREEN}HTML report generated: $HTML_DIR/index.html${NC}"
    else
        echo -e "${YELLOW}cppcheck-htmlreport not found. Install with: pip install cppcheck-htmlreport${NC}"
    fi
fi

# Generate summary report
SUMMARY_FILE="$REPORTS_DIR/summary.txt"
echo -e "\n${BLUE}Generating summary report...${NC}"
{
    echo "LPZRobots Cppcheck Analysis Summary"
    echo "==================================="
    echo "Date: $(date)"
    echo "Cppcheck Version: $CPPCHECK_VERSION"
    echo "Analysis Mode: $([ "$QUICK_MODE" = true ] && echo "Quick" || echo "Full")"
    echo "Incremental: $INCREMENTAL"
    echo "CTU Enabled: $ENABLE_CTU"
    echo "Threads: $THREADS"
    echo ""
    echo "Component Analysis Results"
    echo "--------------------------"
    printf "%-15s %6s %8s %6s %11s %12s %11s\n" "Component" "Errors" "Warnings" "Style" "Performance" "Portability" "Information"
    echo "--------------------------------------------------------------------------------"
    
    TOTAL_ERRORS=0
    TOTAL_WARNINGS=0
    
    while IFS=: read -r component errors warnings style performance portability information; do
        printf "%-15s %6d %8d %6d %11d %12d %11d\n" \
            "$component" "$errors" "$warnings" "$style" "$performance" "$portability" "$information"
        TOTAL_ERRORS=$((TOTAL_ERRORS + errors))
        TOTAL_WARNINGS=$((TOTAL_WARNINGS + warnings))
    done < "$OUTPUT_DIR/metrics.txt"
    
    echo "--------------------------------------------------------------------------------"
    printf "%-15s %6d %8d\n" "TOTAL" "$TOTAL_ERRORS" "$TOTAL_WARNINGS"
    echo ""
    
    # Performance metrics
    end_time=$(date +%s)
    duration=$((end_time - start_time))
    echo "Performance Metrics"
    echo "-------------------"
    echo "Total analysis time: ${duration}s"
    echo "Cache directory size: $(du -sh "$CACHE_DIR" 2>/dev/null | cut -f1 || echo "N/A")"
    
} > "$SUMMARY_FILE"

# Display summary
cat "$SUMMARY_FILE"

# Quality gate check
echo -e "\n${YELLOW}=== Quality Gate ===${NC}"
if [ $TOTAL_ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ No errors found${NC}"
    EXIT_CODE=0
else
    echo -e "${RED}✗ $TOTAL_ERRORS errors found${NC}"
    EXIT_CODE=1
fi

if [ $TOTAL_WARNINGS -gt 100 ]; then
    echo -e "${YELLOW}⚠ Warning threshold exceeded (>100)${NC}"
    [ $EXIT_CODE -eq 0 ] && EXIT_CODE=2
else
    echo -e "${GREEN}✓ Warnings within threshold (<= 100)${NC}"
fi

echo -e "\n${GREEN}Analysis complete!${NC}"
echo -e "Reports saved in: ${BLUE}$REPORTS_DIR${NC}"
echo -e "Cache directory: ${BLUE}$CACHE_DIR${NC}"
echo -e "Summary: ${BLUE}$SUMMARY_FILE${NC}"

# Recommendations
if [ $TOTAL_ERRORS -gt 0 ] || [ $TOTAL_WARNINGS -gt 50 ]; then
    echo -e "\n${YELLOW}Recommendations:${NC}"
    echo "1. Review error reports in: $REPORTS_DIR"
    echo "2. Use inline suppressions for false positives: // cppcheck-suppress [warning-id]"
    echo "3. Update $SUPPRESSIONS_FILE for systematic suppressions"
    [ "$GENERATE_HTML" != true ] && echo "4. Run with --html for easier navigation of results"
fi

exit $EXIT_CODE