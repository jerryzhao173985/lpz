#!/bin/bash
# Script to systematically fix all C-style casts in the codebase

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${GREEN}=== Fixing C-Style Casts Systematically ===${NC}"

# Function to fix casts in a component
fix_casts_in_component() {
    local component=$1
    local path=$2
    
    echo -e "\n${YELLOW}Processing $component...${NC}"
    
    # Count C-style casts before
    local before_count=$(find "$path" \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec grep -h "([A-Za-z_].*\*)" {} \; | grep -v "static_cast" | grep -v "reinterpret_cast" | grep -v "dynamic_cast" | grep -v "const_cast" | wc -l)
    
    if [ $before_count -gt 0 ]; then
        echo "  C-style casts found: $before_count"
        
        # Run the Python script
        python3 "$SCRIPT_DIR/replace_cstyle_casts.py" "$path"
        
        # Count after
        local after_count=$(find "$path" \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -exec grep -h "([A-Za-z_].*\*)" {} \; | grep -v "static_cast" | grep -v "reinterpret_cast" | grep -v "dynamic_cast" | grep -v "const_cast" | wc -l)
        
        echo "  Remaining C-style casts: $after_count"
        echo "  Fixed: $((before_count - after_count)) casts"
    else
        echo "  No C-style casts found"
    fi
}

# Process each component
fix_casts_in_component "selforg/controller" "$PROJECT_ROOT/selforg/controller"
fix_casts_in_component "selforg/wirings" "$PROJECT_ROOT/selforg/wirings"
fix_casts_in_component "selforg/utils" "$PROJECT_ROOT/selforg/utils"
fix_casts_in_component "selforg/statistictools" "$PROJECT_ROOT/selforg/statistictools"
fix_casts_in_component "ode_robots/robots" "$PROJECT_ROOT/ode_robots/robots"
fix_casts_in_component "ode_robots/motors" "$PROJECT_ROOT/ode_robots/motors"
fix_casts_in_component "ode_robots/sensors" "$PROJECT_ROOT/ode_robots/sensors"

echo -e "\n${GREEN}=== C-Style Cast Fixing Complete ===${NC}"

# Generate summary
total_remaining=$(find "$PROJECT_ROOT" \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -path "*/selforg/*" -o -path "*/ode_robots/*" | xargs grep -h "([A-Za-z_].*\*)" | grep -v "static_cast" | grep -v "reinterpret_cast" | grep -v "dynamic_cast" | grep -v "const_cast" | wc -l)

echo "Total remaining C-style casts in project: $total_remaining"