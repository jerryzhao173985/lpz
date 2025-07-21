#!/bin/bash
# Full comprehensive cppcheck analysis with all relevant options
# This is the most thorough analysis - use for complete code review

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Comprehensive Cppcheck Analysis for LPZRobots ===${NC}"
echo "This will perform a thorough analysis considering:"
echo "- Function call graphs and dependencies"
echo "- Class inheritance and member usage"
echo "- Cross-translation-unit analysis"
echo "- C++17 compliance and modernization"
echo ""

# Ensure we have compile_commands.json
if [ ! -f "build/cppcheck/compile_commands.json" ]; then
    echo -e "${YELLOW}Generating compile_commands.json...${NC}"
    cmake -B build/cppcheck \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_GUI_TOOLS=OFF \
        -DBUILD_TESTS=OFF
fi

# Create comprehensive output directory
OUTPUT_DIR="build/cppcheck_reports/comprehensive_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$OUTPUT_DIR"

# Create detailed suppression file for this run
cat > "$OUTPUT_DIR/suppressions.txt" << 'EOF'
# External library paths
*:/opt/homebrew/*
*:/usr/include/*
*:/Applications/Xcode.app/*
*:*/Qt/*
*:*/qt/*
*:*/OpenSceneGraph/*
*:*/osg/*

# Third-party code
*:opende/OPCODE/*
*:opende/ou/*
*:opende/tests/*
*:build/*
*:*/tests/*
*:*/examples/*

# Qt generated files
*:moc_*
*:ui_*
*:qrc_*

# Specific suppressions for known issues
missingIncludeSystem
unmatchedSuppression
EOF

# The comprehensive cppcheck command
echo -e "\n${GREEN}Running comprehensive analysis...${NC}"
echo "Output directory: $OUTPUT_DIR"

# Main analysis command with all bells and whistles
cppcheck \
    --project=build/cppcheck/compile_commands.json \
    --enable=all \
    --std=c++17 \
    --language=c++ \
    --platform=unix64 \
    --force \
    --inconclusive \
    --inline-suppr \
    --suppress-xml=$OUTPUT_DIR/suppressions.txt \
    --max-ctu-depth=10 \
    --max-configs=20 \
    --check-level=exhaustive \
    --library=std,posix,qt \
    --addon=cert,misra \
    --bug-hunting \
    --bug-hunting-check-function-max-time=60 \
    --performance-valueflow-max-iterations=10 \
    --template='{file}:{line}:{column}: [{severity}] ({id}) {message}' \
    --template-location='  --> {file}:{line}:{column}: {info}' \
    --output-file=$OUTPUT_DIR/full_analysis.txt \
    --xml --xml-version=2 \
    --report-progress \
    -j$(nproc) \
    --cppcheck-build-dir=build/cppcheck/.cppcheck \
    -DMAC \
    -DMAC_ARM64 \
    -D__cplusplus=201703L \
    -DNDEBUG \
    -I include \
    -I selforg \
    -I selforg/include \
    -I ode_robots \
    -I ode_robots/include \
    -I ga_tools \
    -I ga_tools/include \
    -I opende \
    -I /opt/homebrew/include \
    selforg \
    ode_robots \
    ga_tools \
    ecbrobots \
    real_robots \
    guilogger/src \
    matrixviz/src \
    configurator \
    2> $OUTPUT_DIR/full_analysis.xml \
    | tee $OUTPUT_DIR/progress.log

echo -e "\n${GREEN}Generating specialized reports...${NC}"

# Extract specific issue categories
echo "Extracting C++17 modernization opportunities..."
grep -E "(modernize-|performance-|readability-)" $OUTPUT_DIR/full_analysis.txt > $OUTPUT_DIR/modernization.txt || true

echo "Extracting memory safety issues..."
grep -E "(memleak|resourceLeak|doubleFree|invalidLifetime|danglingReference|nullPointer|uninitvar)" $OUTPUT_DIR/full_analysis.txt > $OUTPUT_DIR/memory_safety.txt || true

echo "Extracting performance issues..."
grep -E "(performance:|passedByValue|postfixOperator|redundantCopy|inefficientAlgorithm)" $OUTPUT_DIR/full_analysis.txt > $OUTPUT_DIR/performance.txt || true

echo "Extracting potential bugs..."
grep -E "(error:|warning:|arrayIndexOutOfBounds|invalidContainer|wrongPrintfScanfArgNum)" $OUTPUT_DIR/full_analysis.txt > $OUTPUT_DIR/bugs.txt || true

# Generate statistics
echo -e "\n${GREEN}Generating statistics...${NC}"

cat > $OUTPUT_DIR/STATISTICS.md << EOF
# Cppcheck Analysis Statistics
Generated: $(date)

## Overall Summary
Total issues: $(wc -l < $OUTPUT_DIR/full_analysis.txt)

### By Severity
- Errors: $(grep -c "\[error\]" $OUTPUT_DIR/full_analysis.txt || echo 0)
- Warnings: $(grep -c "\[warning\]" $OUTPUT_DIR/full_analysis.txt || echo 0)
- Style: $(grep -c "\[style\]" $OUTPUT_DIR/full_analysis.txt || echo 0)
- Performance: $(grep -c "\[performance\]" $OUTPUT_DIR/full_analysis.txt || echo 0)
- Portability: $(grep -c "\[portability\]" $OUTPUT_DIR/full_analysis.txt || echo 0)
- Information: $(grep -c "\[information\]" $OUTPUT_DIR/full_analysis.txt || echo 0)

### By Category
- C++17 Modernization: $(wc -l < $OUTPUT_DIR/modernization.txt || echo 0)
- Memory Safety: $(wc -l < $OUTPUT_DIR/memory_safety.txt || echo 0)
- Performance: $(wc -l < $OUTPUT_DIR/performance.txt || echo 0)
- Bugs: $(wc -l < $OUTPUT_DIR/bugs.txt || echo 0)

## Top Issues by Type
EOF

echo -e "\n### Most Common Issues" >> $OUTPUT_DIR/STATISTICS.md
cat $OUTPUT_DIR/full_analysis.txt | \
    grep -oE '\([a-zA-Z0-9]+\)' | \
    sort | uniq -c | sort -nr | head -20 >> $OUTPUT_DIR/STATISTICS.md

echo -e "\n### Files with Most Issues" >> $OUTPUT_DIR/STATISTICS.md
cat $OUTPUT_DIR/full_analysis.txt | \
    grep -oE '^[^:]+\.cpp' | \
    sort | uniq -c | sort -nr | head -20 >> $OUTPUT_DIR/STATISTICS.md

# Generate HTML report if available
if command -v cppcheck-htmlreport &> /dev/null; then
    echo -e "\n${GREEN}Generating HTML report...${NC}"
    cppcheck-htmlreport \
        --file=$OUTPUT_DIR/full_analysis.xml \
        --report-dir=$OUTPUT_DIR/html \
        --source-dir=. \
        --title="LPZRobots Comprehensive Analysis"
    echo "HTML report: $OUTPUT_DIR/html/index.html"
fi

# Create actionable summary
cat > $OUTPUT_DIR/ACTION_ITEMS.md << EOF
# Actionable Items from Cppcheck Analysis

## Critical Issues (Fix Immediately)
EOF

grep -E "\[error\]" $OUTPUT_DIR/full_analysis.txt | head -10 >> $OUTPUT_DIR/ACTION_ITEMS.md || echo "No critical errors found." >> $OUTPUT_DIR/ACTION_ITEMS.md

cat >> $OUTPUT_DIR/ACTION_ITEMS.md << EOF

## High Priority (Fix Soon)
EOF

grep -E "\[warning\]" $OUTPUT_DIR/full_analysis.txt | grep -E "(nullPointer|uninitvar|memleak)" | head -10 >> $OUTPUT_DIR/ACTION_ITEMS.md || echo "No high priority warnings found." >> $OUTPUT_DIR/ACTION_ITEMS.md

cat >> $OUTPUT_DIR/ACTION_ITEMS.md << EOF

## C++17 Migration Opportunities
EOF

grep -E "(useAuto|passedByValue|useStlAlgorithm|modernize)" $OUTPUT_DIR/full_analysis.txt | head -10 >> $OUTPUT_DIR/ACTION_ITEMS.md || echo "No obvious modernization opportunities found." >> $OUTPUT_DIR/ACTION_ITEMS.md

# Final summary
echo -e "\n${GREEN}=== Analysis Complete ===${NC}"
echo "Full report: $OUTPUT_DIR/"
echo ""
echo "Key files generated:"
echo "  - full_analysis.txt: Complete text output"
echo "  - full_analysis.xml: XML for further processing"
echo "  - STATISTICS.md: Statistical summary"
echo "  - ACTION_ITEMS.md: Prioritized fixes"
echo "  - modernization.txt: C++17 opportunities"
echo "  - memory_safety.txt: Memory issues"
echo "  - performance.txt: Performance improvements"
echo "  - bugs.txt: Potential bugs"

if [ -d "$OUTPUT_DIR/html" ]; then
    echo "  - html/index.html: Interactive HTML report"
fi

echo -e "\n${YELLOW}Quick Summary:${NC}"
tail -20 $OUTPUT_DIR/STATISTICS.md

echo -e "\n${GREEN}Next steps:${NC}"
echo "1. Review ACTION_ITEMS.md for prioritized fixes"
echo "2. Check error-level issues first"
echo "3. Use specialized scripts for focused analysis"
echo "4. Consider using clang-tidy for additional checks"