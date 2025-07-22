#!/bin/bash
# Setup script for Cppcheck Dashboard Deployment
# This script helps configure the GitHub Pages deployment for cppcheck dashboards

set -e

echo "🚀 Cppcheck Dashboard Deployment Setup"
echo "====================================="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if we're in the repository root
if [ ! -f "Makefile" ] || [ ! -d "tools/cppcheck" ]; then
    echo -e "${RED}Error: This script must be run from the LPZRobots repository root${NC}"
    exit 1
fi

echo "📋 Prerequisites Check"
echo "--------------------"

# Check for required tools
check_tool() {
    if command -v $1 &> /dev/null; then
        echo -e "✅ $1 found"
        return 0
    else
        echo -e "❌ $1 not found"
        return 1
    fi
}

all_tools_found=true
check_tool python3 || all_tools_found=false
check_tool git || all_tools_found=false
check_tool cppcheck || all_tools_found=false

if [ "$all_tools_found" = false ]; then
    echo -e "\n${RED}Please install missing tools before continuing${NC}"
    exit 1
fi

echo -e "\n📁 Setting up directory structure"
echo "--------------------------------"

# Create necessary directories
mkdir -p tools/cppcheck/metrics
mkdir -p tools/cppcheck/reports/deploy
mkdir -p .github/workflows

echo "✅ Directory structure created"

echo -e "\n🔧 Installing Python dependencies"
echo "--------------------------------"

# Check if pip is available
if command -v pip3 &> /dev/null; then
    echo "Installing required Python packages..."
    pip3 install --user pygments plotly pandas jinja2 || {
        echo -e "${YELLOW}Warning: Some Python packages may need to be installed manually${NC}"
    }
else
    echo -e "${YELLOW}pip3 not found. Please install Python packages manually:${NC}"
    echo "  pip3 install pygments plotly pandas jinja2"
fi

echo -e "\n📝 Workflow Installation"
echo "----------------------"

# Check if workflow already exists
if [ -f ".github/workflows/cppcheck-dashboard.yml" ]; then
    echo -e "${YELLOW}Workflow already exists. Skipping...${NC}"
else
    if [ -f "tools/cppcheck/workflows/cppcheck-dashboard.yml" ]; then
        echo "Copying workflow file to .github/workflows/"
        cp tools/cppcheck/workflows/cppcheck-dashboard.yml .github/workflows/
        echo -e "${GREEN}✅ Workflow installed${NC}"
        echo
        echo -e "${YELLOW}IMPORTANT: You need to commit and push this workflow file:${NC}"
        echo "  git add .github/workflows/cppcheck-dashboard.yml"
        echo "  git commit -m 'Add cppcheck dashboard deployment workflow'"
        echo "  git push"
    else
        echo -e "${RED}Workflow source file not found${NC}"
    fi
fi

echo -e "\n🔐 GitHub Configuration"
echo "---------------------"

# Get repository information
if git remote get-url origin &> /dev/null; then
    REPO_URL=$(git remote get-url origin)
    REPO_URL=${REPO_URL#*github.com[/:]}
    REPO_URL=${REPO_URL%.git}
    
    IFS='/' read -r OWNER REPO <<< "$REPO_URL"
    
    echo "Repository: $OWNER/$REPO"
    echo
    echo "📌 Next Steps:"
    echo "1. Enable GitHub Pages:"
    echo "   - Go to: https://github.com/$OWNER/$REPO/settings/pages"
    echo "   - Source: Deploy from a branch"
    echo "   - Branch: gh-pages (will be created automatically)"
    echo
    echo "2. Set workflow permissions:"
    echo "   - Go to: https://github.com/$OWNER/$REPO/settings/actions"
    echo "   - Workflow permissions: Read and write permissions"
    echo
    echo "3. After setup, your dashboard will be available at:"
    echo "   https://$OWNER.github.io/$REPO/"
else
    echo -e "${YELLOW}Could not determine repository information${NC}"
    echo "Please manually configure GitHub Pages in your repository settings"
fi

echo -e "\n🧪 Testing Setup"
echo "--------------"

# Test analysis
echo "Running quick test analysis..."
if python3 tools/cppcheck/scripts/analyze.py --profile quick_check --format json --quiet; then
    echo -e "${GREEN}✅ Analysis successful${NC}"
    
    # Test dashboard generation
    echo "Testing dashboard generation..."
    if python3 tools/cppcheck/scripts/deploy_dashboard.py --deploy-dir test_deploy; then
        echo -e "${GREEN}✅ Dashboard generation successful${NC}"
        rm -rf test_deploy
    else
        echo -e "${RED}❌ Dashboard generation failed${NC}"
    fi
else
    echo -e "${RED}❌ Analysis failed${NC}"
fi

echo -e "\n📊 Optional: Initialize Metrics Database"
echo "--------------------------------------"

read -p "Would you like to initialize the metrics database? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Initializing metrics database..."
    python3 tools/cppcheck/scripts/metrics.py init || {
        echo -e "${YELLOW}Metrics database initialization skipped${NC}"
    }
fi

echo -e "\n✨ Setup Complete!"
echo "=================="
echo
echo "Summary of actions needed:"
echo "1. ✅ Directory structure created"
echo "2. ✅ Python dependencies checked"
if [ ! -f ".github/workflows/cppcheck-dashboard.yml" ]; then
    echo "3. ⚠️  Workflow file needs to be committed and pushed"
else
    echo "3. ✅ Workflow file installed"
fi
echo "4. ⚠️  GitHub Pages needs to be enabled in repository settings"
echo "5. ⚠️  Workflow permissions need to be set to read/write"
echo
echo "Once configured, the dashboard will automatically deploy on:"
echo "- Every push to main branch"
echo "- Every pull request (with PR comment)"
echo "- Daily at 2 AM UTC (comprehensive analysis)"
echo
echo "For more information, see:"
echo "  tools/cppcheck/docs/DASHBOARD_DEPLOYMENT_GUIDE.md"