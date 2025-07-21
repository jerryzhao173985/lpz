#!/bin/bash
# Focused C++17 migration analysis with cppcheck
# This script identifies specific C++17 modernization opportunities

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
NC='\033[0m'

echo -e "${BLUE}=== C++17 Migration Analysis for LPZRobots ===${NC}"
echo ""

# Ensure compile_commands.json exists
if [ ! -f "build/cppcheck/compile_commands.json" ]; then
    echo -e "${YELLOW}Generating compile_commands.json...${NC}"
    cmake -B build/cppcheck -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_GUI_TOOLS=OFF
fi

# Create reports directory
REPORT_DIR="build/cppcheck_reports/cpp17_migration"
mkdir -p "$REPORT_DIR"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

# Function to analyze specific modernization pattern
analyze_pattern() {
    local pattern_name="$1"
    local pattern_desc="$2"
    local cppcheck_args="$3"
    local grep_pattern="$4"
    
    echo -e "\n${GREEN}Analyzing: $pattern_desc${NC}"
    
    local output_file="$REPORT_DIR/${pattern_name}_${TIMESTAMP}.txt"
    
    cppcheck \
        --project=build/cppcheck/compile_commands.json \
        --enable=style,performance,portability,modernize \
        --std=c++17 \
        --language=c++ \
        --suppress=missingInclude \
        --suppress=missingIncludeSystem \
        --suppress=unmatchedSuppression \
        --template='{file}:{line}: {message}' \
        --quiet \
        $cppcheck_args \
        2>&1 | grep -E "$grep_pattern" > "$output_file" || true
    
    local count=$(wc -l < "$output_file" | tr -d ' ')
    echo "  Found $count occurrences"
    
    # Show first 5 examples
    if [ "$count" -gt 0 ]; then
        echo "  Examples:"
        head -5 "$output_file" | sed 's/^/    /'
    fi
}

# 1. NULL to nullptr migration
echo -e "\n${MAGENTA}1. NULL to nullptr Migration${NC}"
grep -rn "NULL\|0 *==" --include="*.cpp" --include="*.h" --include="*.hpp" \
    selforg ode_robots ga_tools 2>/dev/null | \
    grep -v "NOTNULL\|NULL_" | \
    head -20 > "$REPORT_DIR/null_to_nullptr_${TIMESTAMP}.txt" || true
echo "Found $(wc -l < "$REPORT_DIR/null_to_nullptr_${TIMESTAMP}.txt" | tr -d ' ') potential NULL usages"

# 2. typedef to using
echo -e "\n${MAGENTA}2. typedef to using Migration${NC}"
grep -rn "^[[:space:]]*typedef" --include="*.cpp" --include="*.h" --include="*.hpp" \
    selforg ode_robots ga_tools 2>/dev/null | \
    head -20 > "$REPORT_DIR/typedef_to_using_${TIMESTAMP}.txt" || true
echo "Found $(wc -l < "$REPORT_DIR/typedef_to_using_${TIMESTAMP}.txt" | tr -d ' ') typedef declarations"

# 3. auto opportunities
analyze_pattern "auto_usage" \
    "auto usage opportunities" \
    "" \
    "useAuto"

# 4. STL algorithm usage
analyze_pattern "stl_algorithms" \
    "STL algorithm opportunities" \
    "" \
    "useStlAlgorithm"

# 5. Pass by value issues
analyze_pattern "pass_by_value" \
    "Pass by const reference opportunities" \
    "" \
    "passedByValue"

# 6. Initialization list usage
analyze_pattern "init_lists" \
    "Initialization list opportunities" \
    "" \
    "useInitializationList|initializerList"

# 7. Range-based for loops
echo -e "\n${MAGENTA}7. Range-based for loop opportunities${NC}"
grep -rn "for.*\(.*int.*=.*0.*;" --include="*.cpp" --include="*.h" --include="*.hpp" \
    selforg ode_robots ga_tools 2>/dev/null | \
    grep -E "\.size\(\)|\.length\(\)" | \
    head -20 > "$REPORT_DIR/range_for_${TIMESTAMP}.txt" || true
echo "Found $(wc -l < "$REPORT_DIR/range_for_${TIMESTAMP}.txt" | tr -d ' ') traditional for loops that could use range-based for"

# 8. Smart pointer opportunities
echo -e "\n${MAGENTA}8. Smart pointer opportunities${NC}"
grep -rn "new\|delete" --include="*.cpp" --include="*.h" --include="*.hpp" \
    selforg ode_robots ga_tools 2>/dev/null | \
    grep -v "placement new\|operator new\|operator delete" | \
    head -20 > "$REPORT_DIR/smart_pointers_${TIMESTAMP}.txt" || true
echo "Found $(wc -l < "$REPORT_DIR/smart_pointers_${TIMESTAMP}.txt" | tr -d ' ') raw new/delete operations"

# 9. constexpr opportunities
analyze_pattern "constexpr" \
    "constexpr opportunities" \
    "" \
    "constStatement|constVariable"

# 10. override specifier
echo -e "\n${MAGENTA}10. Missing override specifiers${NC}"
cppcheck \
    --project=build/cppcheck/compile_commands.json \
    --enable=style \
    --std=c++17 \
    --suppress=missingInclude \
    --template='{file}:{line}: Add override to {message}' \
    --quiet \
    2>&1 | grep -i "override" > "$REPORT_DIR/override_${TIMESTAMP}.txt" || true
echo "Found $(wc -l < "$REPORT_DIR/override_${TIMESTAMP}.txt" | tr -d ' ') missing override specifiers"

# Generate summary report
SUMMARY="$REPORT_DIR/MIGRATION_SUMMARY_${TIMESTAMP}.md"
cat > "$SUMMARY" << EOF
# C++17 Migration Analysis Summary
Generated: $(date)

## Overview
This report identifies C++17 modernization opportunities in the LPZRobots codebase.

## Findings by Category

### High Priority (Affects API/ABI)
1. **NULL to nullptr**: $(wc -l < "$REPORT_DIR/null_to_nullptr_${TIMESTAMP}.txt" | tr -d ' ') occurrences
2. **Missing override**: $(wc -l < "$REPORT_DIR/override_${TIMESTAMP}.txt" | tr -d ' ') occurrences
3. **typedef to using**: $(wc -l < "$REPORT_DIR/typedef_to_using_${TIMESTAMP}.txt" | tr -d ' ') occurrences

### Medium Priority (Performance/Safety)
4. **Raw new/delete**: $(wc -l < "$REPORT_DIR/smart_pointers_${TIMESTAMP}.txt" | tr -d ' ') occurrences
5. **Pass by value**: $(grep -c "passedByValue" "$REPORT_DIR/pass_by_value_${TIMESTAMP}.txt" 2>/dev/null || echo 0) occurrences
6. **STL algorithms**: $(grep -c "useStlAlgorithm" "$REPORT_DIR/stl_algorithms_${TIMESTAMP}.txt" 2>/dev/null || echo 0) opportunities

### Low Priority (Style/Modernization)
7. **auto usage**: $(grep -c "useAuto" "$REPORT_DIR/auto_usage_${TIMESTAMP}.txt" 2>/dev/null || echo 0) opportunities
8. **Range-based for**: $(wc -l < "$REPORT_DIR/range_for_${TIMESTAMP}.txt" | tr -d ' ') opportunities
9. **Initialization lists**: $(grep -c "useInitializationList" "$REPORT_DIR/init_lists_${TIMESTAMP}.txt" 2>/dev/null || echo 0) opportunities
10. **constexpr**: $(grep -c "const" "$REPORT_DIR/constexpr_${TIMESTAMP}.txt" 2>/dev/null || echo 0) opportunities

## Recommended Migration Order

1. **Phase 1: Critical Safety**
   - Replace NULL with nullptr
   - Add override specifiers
   - Fix pass-by-value for large objects

2. **Phase 2: Type System**
   - Convert typedef to using
   - Add const correctness
   - Use auto for complex types

3. **Phase 3: Resource Management**
   - Replace raw pointers with smart pointers
   - Use RAII consistently
   - Eliminate manual memory management

4. **Phase 4: Performance**
   - Use STL algorithms
   - Apply constexpr where possible
   - Optimize loops with range-based for

## Component-Specific Issues

### selforg
EOF

# Add component-specific analysis
for component in selforg ode_robots ga_tools; do
    echo -e "\n### $component" >> "$SUMMARY"
    echo "Files with most issues:" >> "$SUMMARY"
    cat "$REPORT_DIR"/*_${TIMESTAMP}.txt 2>/dev/null | \
        grep "^$component/" | \
        cut -d: -f1 | \
        sort | uniq -c | \
        sort -nr | head -5 >> "$SUMMARY" || echo "  No significant issues found" >> "$SUMMARY"
done

echo -e "\n## Next Steps\n" >> "$SUMMARY"
echo "1. Run \`scripts/fix_nullptr.sh\` to automatically replace NULL with nullptr" >> "$SUMMARY"
echo "2. Run \`scripts/add_override.sh\` to add missing override specifiers" >> "$SUMMARY"
echo "3. Review smart pointer migration opportunities manually" >> "$SUMMARY"
echo "4. Use clang-tidy for additional modernization checks" >> "$SUMMARY"

# Show summary
echo -e "\n${GREEN}=== Migration Summary ===${NC}"
echo "Reports saved to: $REPORT_DIR"
echo "Summary report: $SUMMARY"

echo -e "\n${YELLOW}Top modernization priorities:${NC}"
echo "1. NULL → nullptr: $(wc -l < "$REPORT_DIR/null_to_nullptr_${TIMESTAMP}.txt" | tr -d ' ') occurrences"
echo "2. Add override: $(wc -l < "$REPORT_DIR/override_${TIMESTAMP}.txt" | tr -d ' ') occurrences"
echo "3. typedef → using: $(wc -l < "$REPORT_DIR/typedef_to_using_${TIMESTAMP}.txt" | tr -d ' ') occurrences"
echo "4. Raw pointers → smart pointers: $(wc -l < "$REPORT_DIR/smart_pointers_${TIMESTAMP}.txt" | tr -d ' ') occurrences"

echo -e "\n${GREEN}To view detailed reports:${NC}"
echo "cat $SUMMARY"