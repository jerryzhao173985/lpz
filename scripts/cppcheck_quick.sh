#!/bin/bash
# Quick cppcheck for specific components or focused analysis

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Default component
COMPONENT="${1:-selforg}"
FOCUS="${2:-all}"

echo -e "${BLUE}=== Quick Cppcheck Analysis: $COMPONENT ===${NC}"

# Ensure compile_commands.json exists
if [ ! -f "build/cppcheck/compile_commands.json" ]; then
    echo -e "${YELLOW}Generating compile_commands.json...${NC}"
    cmake -B build/cppcheck -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_GUI_TOOLS=OFF
fi

# Create output directory
mkdir -p build/cppcheck_reports

case "$FOCUS" in
    "cpp17")
        echo -e "${GREEN}Checking C++17 modernization opportunities...${NC}"
        cppcheck \
            --project=build/cppcheck/compile_commands.json \
            --enable=style,performance,portability \
            --std=c++17 \
            --suppress=missingInclude \
            --suppress=unusedFunction \
            --template='{file}:{line}: Use {message}' \
            --quiet \
            2>&1 | grep -E "(useAuto|useStlAlgorithm|passedByValue|useInitializationList|modernize)" || true
        ;;
    
    "memory")
        echo -e "${GREEN}Checking memory issues...${NC}"
        cppcheck \
            --project=build/cppcheck/compile_commands.json \
            --enable=warning,error \
            --std=c++17 \
            --suppress=missingInclude \
            --template='{file}:{line}: [{severity}] {message}' \
            --quiet \
            2>&1 | grep -E "(leak|delete|free|malloc|new\[|nullPointer|uninit)" || true
        ;;
    
    "safety")
        echo -e "${GREEN}Checking safety issues...${NC}"
        cppcheck \
            --project=build/cppcheck/compile_commands.json \
            --enable=warning,error \
            --std=c++17 \
            --suppress=missingInclude \
            --rule-file=- <<< '
<?xml version="1.0"?>
<rules>
    <rule>
        <pattern>strcpy|strcat|sprintf|gets</pattern>
        <id>unsafeFunction</id>
        <severity>error</severity>
        <summary>Unsafe C function used</summary>
    </rule>
</rules>' \
            --template='{file}:{line}: [{severity}] {message}'
        ;;
    
    "perf")
        echo -e "${GREEN}Checking performance issues...${NC}"
        cppcheck \
            --project=build/cppcheck/compile_commands.json \
            --enable=performance \
            --std=c++17 \
            --suppress=missingInclude \
            --template='{file}:{line}: {message}' \
            --quiet \
            2>&1 | grep -v "missingInclude"
        ;;
    
    *)
        echo -e "${GREEN}Running comprehensive check...${NC}"
        cppcheck \
            --project=build/cppcheck/compile_commands.json \
            --enable=all \
            --std=c++17 \
            --suppress=missingInclude \
            --suppress=unusedFunction \
            --suppress=missingIncludeSystem \
            --template='{file}:{line}: [{severity}] ({id}) {message}' \
            --quiet
        ;;
esac

echo -e "\n${GREEN}Quick analysis complete!${NC}"
echo -e "For full analysis, run: ${BLUE}./scripts/cppcheck_analysis.sh${NC}"