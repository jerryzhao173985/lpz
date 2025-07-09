#!/usr/bin/env python3
"""
Extract member initialization fixes from the patch.
Looking for constructor initialization list improvements.
"""

import re
import sys

def is_member_init_change(lines, start_idx):
    """Check if a hunk contains member initialization changes."""
    for i in range(start_idx, min(start_idx + 50, len(lines))):
        line = lines[i]
        
        # Look for constructor initialization lists
        # Pattern: : member(value), member2(value)
        if ': ' in line and '(' in line and ')' in line:
            # Check if it's in a constructor context
            if line.startswith('-') or line.startswith('+'):
                # Look for initialization patterns
                if re.search(r':\s*\w+\([^)]*\)', line):
                    # Check for 0 to nullptr conversions
                    if (line.startswith('-') and '(0)' in line) or \
                       (line.startswith('+') and '(nullptr)' in line):
                        return True
                    # Check for member initialization additions
                    if line.startswith('+') and not any(lines[j].startswith('-') and ': ' in lines[j] for j in range(max(0, i-5), i)):
                        return True
        
        # Look for in-class member initialization
        # Pattern: member = value; or member{value};
        if line.startswith('+') and ('=' in line or '{' in line) and ';' in line:
            # Check if it's a member variable initialization
            if re.search(r'^\+\s*([\w:]+\s+)?[\w_]+\s*[={].*[;}]', line):
                # Exclude function definitions
                if '(' not in line or ')' not in line.split('=')[0]:
                    return True
                    
    return False

def extract_member_init_changes(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = []
    
    while i < len(lines):
        line = lines[i]
        
        # Found a new file diff
        if line.startswith('diff --git'):
            file_start = i
            file_has_init = False
            current_file = line.split()[2]
            
            # Skip non-source files
            if not any(current_file.endswith(ext) for ext in ['.h', '.hpp', '.cpp', '.cc']):
                # Find next file
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Collect the entire file diff
            j = i + 1
            while j < len(lines) and not lines[j].startswith('diff --git'):
                # Check each hunk in this file
                if lines[j].startswith('@@'):
                    if is_member_init_change(lines, j):
                        file_has_init = True
                j += 1
            
            # If this file has member init changes, include it
            if file_has_init:
                # Add the entire file diff
                for k in range(file_start, j):
                    output_lines.append(lines[k])
                files_with_changes.append(current_file)
            
            i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count specific types of changes
    nullptr_conversions = 0
    member_inits = 0
    
    for line in output_lines:
        if line.startswith('-') and '(0)' in line and ': ' in line:
            nullptr_conversions += 1
        if line.startswith('+') and ('= ' in line or ' = {' in line) and ';' in line:
            member_inits += 1
    
    return nullptr_conversions, member_inits, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_06_member_init_only.patch'
    
    print("Extracting member initialization changes...")
    nullptr_conv, member_init, file_count = extract_member_init_changes(input_file, output_file)
    
    print(f"\nExtracted member initialization changes:")
    print(f"  - nullptr conversions in constructors: {nullptr_conv}")
    print(f"  - Member initializations: {member_init}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")