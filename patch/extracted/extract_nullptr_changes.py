#!/usr/bin/env python3
"""
Extract NULL to nullptr changes from the big patch.
This script carefully identifies and extracts only the relevant changes.
Based on the pattern established in extract_typedef_changes.py
"""

import re
import sys

def is_nullptr_change(lines, start_idx):
    """Check if a hunk contains NULL to nullptr changes."""
    for i in range(start_idx, min(start_idx + 50, len(lines))):
        line = lines[i]
        
        # Skip documentation and comments
        if any(marker in line for marker in ['```', '###', '//', '/*', '*/', 'NOTE:', 'TODO:', '- ', '* ']):
            continue
            
        # Look for NULL removal
        if line.startswith('-') and 'NULL' in line:
            # Skip if NULL is in a string
            if '"' in line and 'NULL' in line[line.find('"'):line.rfind('"')]:
                continue
            # Check if there's a corresponding nullptr addition nearby
            for j in range(max(0, i-5), min(i+5, len(lines))):
                if lines[j].startswith('+') and 'nullptr' in lines[j]:
                    return True
            # Sometimes NULL is replaced in same line
            if '!= NULL' in line or '== NULL' in line or '= NULL' in line:
                return True
                
        # Look for nullptr addition
        if line.startswith('+') and 'nullptr' in line:
            # Check if there's a NULL removal nearby
            for j in range(max(0, i-5), min(i+5, len(lines))):
                if lines[j].startswith('-') and 'NULL' in lines[j]:
                    return True
                    
    return False

def extract_nullptr_changes(input_file, output_file):
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
            file_has_nullptr = False
            current_file = line.split()[2]
            
            # Skip non-source files
            if not any(current_file.endswith(ext) for ext in ['.h', '.hpp', '.cpp', '.cc', '.c']):
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
                    if is_nullptr_change(lines, j):
                        file_has_nullptr = True
                j += 1
            
            # If this file has nullptr changes, include it
            if file_has_nullptr:
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
    
    # Write list of files
    with open('patch/extracted/files_with_nullptr_changes.txt', 'w') as f:
        for file in files_with_changes:
            f.write(f"{file}\n")
    
    # Count the changes
    null_count = 0
    nullptr_count = 0
    for line in output_lines:
        if line.startswith('-') and 'NULL' in line:
            # Count actual NULL occurrences, not in strings or comments
            if not any(marker in line for marker in ['```', '//', '/*', '"NULL"']):
                null_count += line.count('NULL')
        if line.startswith('+') and 'nullptr' in line:
            nullptr_count += line.count('nullptr')
    
    return null_count, nullptr_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_02_null_to_nullptr.patch'
    
    print("Extracting NULL to nullptr changes...")
    null_count, nullptr_count, file_count = extract_nullptr_changes(input_file, output_file)
    
    print(f"Extracted NULL to nullptr changes:")
    print(f"  - NULL removals: {null_count}")
    print(f"  - nullptr additions: {nullptr_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")