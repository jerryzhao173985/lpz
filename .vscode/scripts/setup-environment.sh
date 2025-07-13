#!/bin/bash
# ╔══════════════════════════════════════════════════════════════════╗
# ║           LPZRobots Development Environment Setup                 ║
# ╚══════════════════════════════════════════════════════════════════╝

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Detect OS
OS="unknown"
if [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
fi

echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${BLUE}            LPZRobots Development Environment Setup                 ${NC}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo ""

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check version
check_version() {
    local cmd=$1
    local min_version=$2
    local current_version=$($cmd --version 2>&1 | grep -oE '[0-9]+\.[0-9]+' | head -1)
    
    if [ "$(printf '%s\n' "$min_version" "$current_version" | sort -V | head -n1)" = "$min_version" ]; then
        echo -e "${GREEN}✓${NC} $cmd version $current_version (>= $min_version)"
        return 0
    else
        echo -e "${RED}✗${NC} $cmd version $current_version (< $min_version)"
        return 1
    fi
}

# Check essential tools
echo -e "${YELLOW}Checking essential tools...${NC}"
echo ""

# CMake
if command_exists cmake; then
    check_version cmake 3.20 || echo -e "${RED}  Please update CMake to version 3.20 or later${NC}"
else
    echo -e "${RED}✗ CMake not found${NC}"
    echo "  Install: brew install cmake (macOS) or sudo apt install cmake (Linux)"
fi

# C++ Compiler
if command_exists clang++; then
    echo -e "${GREEN}✓${NC} clang++ found"
elif command_exists g++; then
    echo -e "${GREEN}✓${NC} g++ found"
else
    echo -e "${RED}✗ No C++ compiler found${NC}"
    echo "  Install: xcode-select --install (macOS) or sudo apt install build-essential (Linux)"
fi

# Ninja
if command_exists ninja; then
    echo -e "${GREEN}✓${NC} ninja found"
else
    echo -e "${YELLOW}⚠${NC} ninja not found (optional but recommended)"
    echo "  Install: brew install ninja (macOS) or sudo apt install ninja-build (Linux)"
fi

echo ""
echo -e "${YELLOW}Checking dependencies...${NC}"
echo ""

# Platform-specific checks
if [ "$OS" = "macos" ]; then
    # Check Homebrew
    if command_exists brew; then
        echo -e "${GREEN}✓${NC} Homebrew found"
        
        # Check dependencies
        deps=("qt@6" "open-scene-graph" "gsl" "readline" "llvm")
        missing_deps=()
        
        for dep in "${deps[@]}"; do
            if brew list "$dep" &>/dev/null; then
                echo -e "${GREEN}✓${NC} $dep installed"
            else
                echo -e "${RED}✗${NC} $dep not installed"
                missing_deps+=("$dep")
            fi
        done
        
        if [ ${#missing_deps[@]} -gt 0 ]; then
            echo ""
            echo -e "${YELLOW}Install missing dependencies with:${NC}"
            echo "  brew install ${missing_deps[*]}"
        fi
    else
        echo -e "${RED}✗ Homebrew not found${NC}"
        echo "  Install from https://brew.sh"
    fi
    
elif [ "$OS" = "linux" ]; then
    # Check for package managers
    if command_exists apt-get; then
        echo -e "${GREEN}✓${NC} apt-get found"
        echo ""
        echo -e "${YELLOW}Ensure these packages are installed:${NC}"
        echo "  sudo apt-get install qt6-base-dev libopenscenegraph-dev \\"
        echo "       libgsl-dev libreadline-dev libncurses5-dev \\"
        echo "       libglu1-mesa-dev freeglut3-dev clang-tidy clang-format"
    elif command_exists pacman; then
        echo -e "${GREEN}✓${NC} pacman found"
        echo ""
        echo -e "${YELLOW}Ensure these packages are installed:${NC}"
        echo "  sudo pacman -S qt6-base openscenegraph gsl readline ncurses glu freeglut"
    fi
fi

echo ""
echo -e "${YELLOW}Checking VSCode extensions...${NC}"
echo ""

# Check if code command exists
if command_exists code; then
    # Check essential extensions
    extensions=(
        "ms-vscode.cpptools"
        "ms-vscode.cmake-tools"
    )
    
    installed_extensions=$(code --list-extensions 2>/dev/null || echo "")
    
    for ext in "${extensions[@]}"; do
        if echo "$installed_extensions" | grep -q "$ext"; then
            echo -e "${GREEN}✓${NC} $ext installed"
        else
            echo -e "${RED}✗${NC} $ext not installed"
            echo "  Install with: code --install-extension $ext"
        fi
    done
else
    echo -e "${YELLOW}⚠${NC} VSCode command line tools not found"
    echo "  In VSCode: Cmd+Shift+P → 'Shell Command: Install code command in PATH'"
fi

echo ""
echo -e "${YELLOW}Setting up project...${NC}"
echo ""

# Create build directories
echo -e "${BLUE}Creating build directories...${NC}"
mkdir -p build/{debug,release,dev,dev-sanitizers}
echo -e "${GREEN}✓${NC} Build directories created"

# Generate compile_commands.json link
if [ -f "build/debug/compile_commands.json" ]; then
    ln -sf build/debug/compile_commands.json .
    echo -e "${GREEN}✓${NC} Linked compile_commands.json"
fi

# Set up git hooks (if in git repo)
if [ -d ".git" ]; then
    echo ""
    echo -e "${YELLOW}Setting up git hooks...${NC}"
    
    # Pre-commit hook for formatting
    cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Format staged C++ files
for file in $(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|h|hpp)$'); do
    clang-format -i "$file"
    git add "$file"
done
EOF
    chmod +x .git/hooks/pre-commit
    echo -e "${GREEN}✓${NC} Pre-commit hook installed"
fi

echo ""
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}Setup complete!${NC}"
echo ""
echo "Next steps:"
echo "1. Open VSCode: code lpzrobots.code-workspace"
echo "2. Configure CMake: Cmd+Shift+P → 'CMake: Configure'"
echo "3. Build: Cmd+Shift+B"
echo ""
echo "For more information, see .vscode/SETUP.md"
echo -e "${BLUE}═══════════════════════════════════════════════════════════════════${NC}"