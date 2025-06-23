#!/usr/bin/env python3
"""
Tool to add 'override' keywords to virtual functions in derived classes.
This uses clang's compilation database to properly parse C++ code.
"""

import os
import re
import sys
import subprocess
from pathlib import Path

def find_virtual_functions_needing_override(directory):
    """
    Use cppcheck to find virtual functions that need override keyword.
    """
    print(f"Scanning {directory} for missing override keywords...")
    
    result = subprocess.run(
        ['cppcheck', '--enable=style', '--suppress=missingInclude', 
         '--inline-suppr', '--quiet', directory],
        capture_output=True,
        text=True
    )
    
    # Parse cppcheck output for missing override warnings
    missing_overrides = []
    for line in result.stderr.split('\n'):
        if 'missingOverride' in line:
            # Extract file path and line number
            match = re.match(r'([^:]+):(\d+):\d+: style: The \w+ \'(\w+)\' overrides', line)
            if match:
                filepath = match.group(1)
                line_num = int(match.group(2))
                function_name = match.group(3)
                missing_overrides.append((filepath, line_num, function_name))
    
    return missing_overrides

def add_override_to_function(filepath, line_num, function_name):
    """
    Add override keyword to a specific function.
    """
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    if line_num <= 0 or line_num > len(lines):
        return False
    
    # Get the line (0-indexed)
    line_idx = line_num - 1
    line = lines[line_idx]
    
    # Check if override already exists
    if 'override' in line:
        return False
    
    # Add override before the semicolon or opening brace
    # Handle different patterns:
    # 1. Declaration ending with ;
    # 2. Declaration with { on same line
    # 3. Declaration with = 0; (pure virtual)
    
    if ';' in line:
        # Check for pure virtual (= 0)
        if '= 0' in line or '=0' in line:
            line = re.sub(r'(\s*=\s*0\s*);', r'\1 override;', line)
        else:
            line = re.sub(r'(\s*);', r' override;', line)
    elif '{' in line:
        line = re.sub(r'(\s*)\{', r' override\1{', line)
    else:
        # Multi-line declaration, need to look ahead
        for i in range(line_idx + 1, min(line_idx + 5, len(lines))):
            if ';' in lines[i] or '{' in lines[i]:
                if ';' in lines[i]:
                    if '= 0' in lines[i] or '=0' in lines[i]:
                        lines[i] = re.sub(r'(\s*=\s*0\s*);', r'\1 override;', lines[i])
                    else:
                        lines[i] = re.sub(r'(\s*);', r' override;', lines[i])
                else:
                    lines[i] = re.sub(r'(\s*)\{', r' override\1{', lines[i])
                break
        else:
            return False
    
    lines[line_idx] = line
    
    # Write back
    with open(filepath, 'w') as f:
        f.writelines(lines)
    
    return True

def fix_override_keywords(directory, max_fixes=None):
    """
    Fix missing override keywords in a directory.
    """
    missing_overrides = find_virtual_functions_needing_override(directory)
    
    print(f"Found {len(missing_overrides)} functions missing override keyword")
    
    if max_fixes:
        missing_overrides = missing_overrides[:max_fixes]
        print(f"Limiting to {max_fixes} fixes")
    
    fixed_count = 0
    files_modified = set()
    
    for filepath, line_num, function_name in missing_overrides:
        if add_override_to_function(filepath, line_num, function_name):
            fixed_count += 1
            files_modified.add(filepath)
            print(f"✓ Fixed {function_name} in {filepath}:{line_num}")
        else:
            print(f"✗ Failed to fix {function_name} in {filepath}:{line_num}")
    
    print(f"\n=== Summary ===")
    print(f"Functions fixed: {fixed_count}")
    print(f"Files modified: {len(files_modified)}")
    
    return fixed_count

if __name__ == "__main__":
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = '.'
    
    # Fix in batches to avoid overwhelming changes
    fix_override_keywords(directory, max_fixes=50)