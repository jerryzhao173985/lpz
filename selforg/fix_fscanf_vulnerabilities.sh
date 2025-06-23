#!/bin/bash
# Script to find and fix fscanf vulnerabilities in selforg component
# This fixes buffer overflow vulnerabilities by adding field width limits

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Finding and Fixing fscanf vulnerabilities in selforg ===${NC}"
echo ""

# Create backup directory
BACKUP_DIR="fscanf_backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
echo -e "${YELLOW}Creating backups in: $BACKUP_DIR${NC}"
echo ""

# Function to fix a file
fix_file() {
    local file="$1"
    local backup_file="$BACKUP_DIR/$(basename "$file")"
    
    # Create backup
    cp "$file" "$backup_file"
    
    # Check if file has vulnerable fscanf calls
    if grep -q 'fscanf.*"%s"' "$file"; then
        echo -e "${YELLOW}Processing: $file${NC}"
        
        # Create temporary file
        tmp_file=$(mktemp)
        
        # Process the file line by line
        awk '
        /fscanf.*"%s"/ && /buffer/ && !/"%[0-9]+s"/ {
            # Extract buffer size if possible from nearby char buffer declaration
            # Default to 127 (leaving 1 byte for null terminator in 128-byte buffers)
            buffer_size = 127
            
            # Replace %s with field-width-limited %127s
            gsub(/"%s/, "\"%127s", $0)
            print "// SECURITY FIX: Added field width limit to prevent buffer overflow"
            print $0
            next
        }
        { print }
        ' "$file" > "$tmp_file"
        
        # Replace original file
        mv "$tmp_file" "$file"
        
        echo -e "${GREEN}Fixed vulnerabilities in: $file${NC}"
    fi
}

# Find all vulnerable files
echo -e "${YELLOW}Searching for vulnerable fscanf patterns...${NC}"
echo ""

# Create list of files with issues
vulnerable_files=()

# Find C/C++ files with fscanf
while IFS= read -r -d '' file; do
    if grep -q 'fscanf.*"%s"' "$file" 2>/dev/null; then
        vulnerable_files+=("$file")
    fi
done < <(find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" -o -name "*.h" -o -name "*.hpp" \) -print0)

# Report findings
echo -e "${RED}Found ${#vulnerable_files[@]} files with vulnerable fscanf calls:${NC}"
for file in "${vulnerable_files[@]}"; do
    echo "  - $file"
    # Show the vulnerable lines
    grep -n 'fscanf.*"%s"' "$file" | while IFS= read -r line; do
        echo "    Line: $line"
    done
done
echo ""

# Ask for confirmation
read -p "Do you want to fix these vulnerabilities? (y/n) " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Aborted."
    exit 1
fi

# Fix each file
for file in "${vulnerable_files[@]}"; do
    fix_file "$file"
done

echo ""
echo -e "${GREEN}=== Summary ===${NC}"
echo "Fixed ${#vulnerable_files[@]} files"
echo "Backups saved in: $BACKUP_DIR"
echo ""

# Verify fixes
echo -e "${YELLOW}Verifying fixes...${NC}"
remaining_issues=0
for file in "${vulnerable_files[@]}"; do
    if grep -q 'fscanf.*"%s"' "$file" 2>/dev/null; then
        echo -e "${RED}WARNING: $file still has vulnerable patterns${NC}"
        ((remaining_issues++))
    fi
done

if [ $remaining_issues -eq 0 ]; then
    echo -e "${GREEN}All vulnerabilities have been fixed!${NC}"
else
    echo -e "${RED}$remaining_issues files still have issues${NC}"
fi

echo ""
echo -e "${YELLOW}Recommended next steps:${NC}"
echo "1. Review the changes using: diff -r $BACKUP_DIR ."
echo "2. Compile and test the code to ensure functionality"
echo "3. Consider using safer alternatives like fgets() with sscanf()"
echo "4. Add input validation after parsing"