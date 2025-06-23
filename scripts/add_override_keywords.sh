#!/bin/bash
# Script to add override keywords to virtual functions using clang-tidy

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${GREEN}Adding override keywords to virtual functions...${NC}"

# Function to process a component
process_component() {
    local component=$1
    local path=$2
    
    echo -e "\n${YELLOW}Processing $component...${NC}"
    
    # Count virtual functions without override
    local before_count=$(grep -r "virtual.*(" "$path" --include="*.h" --include="*.hpp" | grep -v "override" | wc -l)
    echo "  Virtual functions without override: $before_count"
    
    if [ $before_count -gt 0 ]; then
        # Run clang-tidy with modernize-use-override check
        find "$path" \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -print0 | \
        while IFS= read -r -d '' file; do
            echo -e "${BLUE}  Processing: ${file#$PROJECT_ROOT/}${NC}"
            clang-tidy "$file" \
                --checks="-*,modernize-use-override" \
                --fix \
                --fix-errors \
                -- -std=c++17 \
                -I"$PROJECT_ROOT/selforg/include" \
                -I"$PROJECT_ROOT/selforg" \
                -I"$PROJECT_ROOT/ode_robots/include" \
                -I/opt/homebrew/include \
                -I/usr/local/include \
                2>/dev/null
        done
        
        # Count after processing
        local after_count=$(grep -r "virtual.*(" "$path" --include="*.h" --include="*.hpp" | grep -v "override" | wc -l)
        echo "  Remaining virtual functions without override: $after_count"
        echo "  Fixed: $((before_count - after_count)) functions"
    else
        echo "  No virtual functions without override found"
    fi
}

# Process each component
process_component "selforg/controller" "$PROJECT_ROOT/selforg/controller"
process_component "selforg/wirings" "$PROJECT_ROOT/selforg/wirings"
process_component "selforg/utils" "$PROJECT_ROOT/selforg/utils"
process_component "ode_robots/robots" "$PROJECT_ROOT/ode_robots/robots"
process_component "ode_robots/obstacles" "$PROJECT_ROOT/ode_robots/obstacles"

echo -e "\n${GREEN}Override keyword addition complete!${NC}"

# Generate summary report
echo -e "\n${GREEN}Generating summary...${NC}"
total_remaining=$(grep -r "virtual.*(" "$PROJECT_ROOT" --include="*.h" --include="*.hpp" | grep -v "override" | grep -v "= 0" | wc -l)
echo "Total remaining virtual functions without override: $total_remaining"

# Show a few examples of remaining cases
echo -e "\n${YELLOW}Sample remaining cases (may be intentional):${NC}"
grep -r "virtual.*(" "$PROJECT_ROOT" --include="*.h" --include="*.hpp" | grep -v "override" | grep -v "= 0" | head -5