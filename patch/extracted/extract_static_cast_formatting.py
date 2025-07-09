#!/usr/bin/env python3
"""
Extract static_cast formatting fixes from big.patch.
Fixes spacing between static_cast<Type> and (value).
"""

import re
import os

def extract_static_cast_formatting(input_file, output_file):
    """Extract lines where static_cast has space before parenthesis."""
    
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    current_file = None
    current_hunk = []
    has_static_cast_fix = False
    files_with_changes = set()
    
    i = 0
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('diff --git'):
            # Process previous file if it had changes
            if has_static_cast_fix and current_hunk:
                output_lines.extend(current_hunk)
                if current_file:
                    files_with_changes.add(current_file)
            
            # Reset for new file
            current_file = line.split()[2].replace('a/', '')
            current_hunk = [line]
            has_static_cast_fix = False
            
        elif line.startswith('@@'):
            # Start of a new hunk
            if has_static_cast_fix:
                output_lines.extend(current_hunk)
            current_hunk = [line]
            has_static_cast_fix = False
            
        else:
            current_hunk.append(line)
            
            # Look for static_cast with space before parenthesis
            if line.startswith('-') and 'static_cast' in line:
                # Pattern: static_cast<Type> (
                if re.search(r'static_cast<[^>]+>\s+\(', line):
                    has_static_cast_fix = True
            elif line.startswith('+') and 'static_cast' in line:
                # Check if it's the fixed version
                if re.search(r'static_cast<[^>]+>\(', line):
                    has_static_cast_fix = True
        
        i += 1
    
    # Don't forget the last file
    if has_static_cast_fix and current_hunk:
        output_lines.extend(current_hunk)
        if current_file:
            files_with_changes.add(current_file)
    
    # Write output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    return len(files_with_changes), count_static_cast_fixes(output_lines)

def count_static_cast_fixes(lines):
    """Count the number of static_cast formatting fixes."""
    count = 0
    for line in lines:
        if line.startswith('-') and re.search(r'static_cast<[^>]+>\s+\(', line):
            count += 1
    return count

def create_find_script(patch_file):
    """Create a script to find all static_cast with space issues in current code."""
    
    script_content = '''#!/bin/bash
# Find all static_cast with space before parenthesis
echo "Searching for static_cast formatting issues..."

# Find in all C++ files
find . -type f \\( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \\) \\
    -not -path "./build/*" \\
    -not -path "./dist/*" \\
    -not -path "./.cache/*" \\
    -exec grep -l 'static_cast<[^>]*> (' {} \\; | sort | uniq

echo ""
echo "To see the actual lines:"
echo 'grep -n "static_cast<[^>]*> (" <filename>'
'''
    
    script_file = 'find_static_cast_spacing.sh'
    with open(script_file, 'w') as f:
        f.write(script_content)
    os.chmod(script_file, 0o755)
    print(f"Created search script: {script_file}")

if __name__ == "__main__":
    input_file = '../big.patch'
    output_file = 'patch_10_static_cast_formatting.patch'
    
    print("Extracting static_cast formatting fixes...")
    
    if not os.path.exists(input_file):
        print(f"Error: {input_file} not found!")
        exit(1)
    
    file_count, fix_count = extract_static_cast_formatting(input_file, output_file)
    
    print(f"\nExtraction complete!")
    print(f"Files with changes: {file_count}")
    print(f"Static cast fixes: {fix_count}")
    print(f"Output written to: {output_file}")
    
    # Create helper script
    create_find_script(output_file)
    
    print("\nNext steps:")
    print("1. Run ./find_static_cast_spacing.sh to find current issues")
    print("2. Review the patch file")
    print("3. Apply with: git apply patch_10_static_cast_formatting.patch")