#!/bin/bash
# Build LPZRobots with AddressSanitizer for memory error detection

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}Building LPZRobots with AddressSanitizer...${NC}"

# Export ASan flags
export CXXFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1"
export LDFLAGS="-fsanitize=address"

# ASan runtime options
export ASAN_OPTIONS="detect_leaks=1:check_initialization_order=1:strict_init_order=1:detect_stack_use_after_return=1:print_stats=1:halt_on_error=0"

# Build components
cd "$PROJECT_ROOT"

echo -e "${YELLOW}Cleaning previous builds...${NC}"
make clean-all

echo -e "${YELLOW}Building selforg with ASan...${NC}"
make selforg

echo -e "${YELLOW}Building ode_robots with ASan...${NC}"
make ode_robots

echo -e "${GREEN}Build complete!${NC}"
echo ""
echo "To run simulations with ASan:"
echo "  export ASAN_OPTIONS=\"$ASAN_OPTIONS\""
echo "  cd ode_robots/simulations/your_simulation"
echo "  ./start"
echo ""
echo "ASan will report memory errors like:"
echo "  - Buffer overflows"
echo "  - Use after free"
echo "  - Memory leaks"
echo "  - Double free"