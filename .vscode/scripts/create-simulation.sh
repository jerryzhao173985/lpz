#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║              Create New LPZRobots Simulation                      ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Default values
TEMPLATE="template_sphererobot"
SIMULATIONS_DIR="ode_robots/simulations"

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}               Create New LPZRobots Simulation                      ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ] || [ ! -d "$SIMULATIONS_DIR" ]; then
    echo -e "${RED}Error: This script must be run from the LPZRobots root directory${NC}"
    exit 1
fi

# Get simulation name
if [ -z "$1" ]; then
    echo -n "Enter simulation name (e.g., my_robot_experiment): "
    read -r SIM_NAME
else
    SIM_NAME="$1"
fi

# Validate name
if [[ ! "$SIM_NAME" =~ ^[a-zA-Z][a-zA-Z0-9_]*$ ]]; then
    echo -e "${RED}Error: Simulation name must start with a letter and contain only letters, numbers, and underscores${NC}"
    exit 1
fi

# Check if already exists
if [ -d "$SIMULATIONS_DIR/$SIM_NAME" ]; then
    echo -e "${RED}Error: Simulation '$SIM_NAME' already exists${NC}"
    exit 1
fi

# Select template
echo ""
echo "Available templates:"
templates=($(find "$SIMULATIONS_DIR" -maxdepth 1 -name "template_*" -type d -exec basename {} \; | sort))

if [ ${#templates[@]} -eq 0 ]; then
    echo -e "${RED}Error: No templates found in $SIMULATIONS_DIR${NC}"
    exit 1
fi

for i in "${!templates[@]}"; do
    echo "  $((i+1)). ${templates[$i]}"
done

echo ""
echo -n "Select template (1-${#templates[@]}) [1]: "
read -r TEMPLATE_CHOICE

if [ -z "$TEMPLATE_CHOICE" ]; then
    TEMPLATE_CHOICE=1
fi

if [[ "$TEMPLATE_CHOICE" =~ ^[0-9]+$ ]] && [ "$TEMPLATE_CHOICE" -ge 1 ] && [ "$TEMPLATE_CHOICE" -le ${#templates[@]} ]; then
    TEMPLATE="${templates[$((TEMPLATE_CHOICE-1))]}"
else
    echo -e "${RED}Error: Invalid template choice${NC}"
    exit 1
fi

echo ""
echo -e "${YELLOW}Creating simulation '$SIM_NAME' from template '$TEMPLATE'...${NC}"

# Copy template
cp -r "$SIMULATIONS_DIR/$TEMPLATE" "$SIMULATIONS_DIR/$SIM_NAME"
echo -e "${GREEN}✓${NC} Copied template files"

# Update main.cpp
MAIN_FILE="$SIMULATIONS_DIR/$SIM_NAME/main.cpp"
if [ -f "$MAIN_FILE" ]; then
    # Replace template name with new simulation name
    sed -i.bak "s/$TEMPLATE/$SIM_NAME/g" "$MAIN_FILE"
    sed -i.bak "s/Template/$(echo $SIM_NAME | sed 's/_/ /g' | awk '{for(i=1;i<=NF;i++)sub(/./,toupper(substr($i,1,1)),$i)}1' | sed 's/ //g')/g" "$MAIN_FILE"
    rm "$MAIN_FILE.bak"
    echo -e "${GREEN}✓${NC} Updated main.cpp"
fi

# Update Makefile if exists
MAKEFILE="$SIMULATIONS_DIR/$SIM_NAME/Makefile"
if [ -f "$MAKEFILE" ]; then
    sed -i.bak "s/$TEMPLATE/$SIM_NAME/g" "$MAKEFILE"
    rm "$MAKEFILE.bak"
    echo -e "${GREEN}✓${NC} Updated Makefile"
fi

# Create or update CMakeLists.txt
CMAKE_FILE="$SIMULATIONS_DIR/$SIM_NAME/CMakeLists.txt"
cat > "$CMAKE_FILE" << EOF
# CMakeLists.txt for $SIM_NAME simulation

# Simulation executable
add_executable($SIM_NAME
    main.cpp
)

# Link required libraries
target_link_libraries($SIM_NAME
    PRIVATE
        lpzrobots::ode_robots
        lpzrobots::selforg
        lpzrobots::opende
        \${OSG_LIBRARIES}
        \${OPENGL_LIBRARIES}
)

# Set properties
set_target_properties($SIM_NAME PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "\${CMAKE_CURRENT_BINARY_DIR}"
    OUTPUT_NAME "start"
)

# Copy any required resources
# file(COPY \${CMAKE_CURRENT_SOURCE_DIR}/resources
#      DESTINATION \${CMAKE_CURRENT_BINARY_DIR})
EOF
echo -e "${GREEN}✓${NC} Created CMakeLists.txt"

# Add to parent CMakeLists.txt
PARENT_CMAKE="$SIMULATIONS_DIR/CMakeLists.txt"
if [ -f "$PARENT_CMAKE" ]; then
    if ! grep -q "add_subdirectory($SIM_NAME)" "$PARENT_CMAKE"; then
        echo "add_subdirectory($SIM_NAME)" >> "$PARENT_CMAKE"
        echo -e "${GREEN}✓${NC} Added to parent CMakeLists.txt"
    fi
fi

# Create a basic README
README_FILE="$SIMULATIONS_DIR/$SIM_NAME/README.md"
cat > "$README_FILE" << EOF
# $SIM_NAME Simulation

This simulation was created from the $TEMPLATE template.

## Description

[Add your simulation description here]

## Usage

\`\`\`bash
# Build
cd build
cmake ..
make $SIM_NAME

# Run
cd ode_robots/simulations/$SIM_NAME
./start

# Run with GUI tools
./start -g 5 -m 10
\`\`\`

## Parameters

[Document your simulation parameters here]

## Notes

[Add any additional notes here]
EOF
echo -e "${GREEN}✓${NC} Created README.md"

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Simulation '$SIM_NAME' created successfully!${NC}"
echo ""
echo "Location: $SIMULATIONS_DIR/$SIM_NAME"
echo ""
echo "Next steps:"
echo "1. Edit main.cpp to implement your simulation"
echo "2. Build with: cmake --build build --target $SIM_NAME"
echo "3. Run with: cd $SIMULATIONS_DIR/$SIM_NAME && ./start"
echo ""
echo "VSCode shortcuts:"
echo "- Build: Cmd+Alt+B (when main.cpp is open)"
echo "- Run: Cmd+R"
echo "- Debug: F5"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"