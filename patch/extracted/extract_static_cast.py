#!/usr/bin/env python3
"""
Extract C-style cast to static_cast conversions.
Focus on actual C-style casts that should be modernized.
"""

import re

def is_static_cast_change(lines, start_idx):
    """Check if a hunk contains C-style cast to static_cast conversions."""
    for i in range(start_idx, min(start_idx + 50, len(lines))):
        line = lines[i]
        
        # Skip documentation and comments
        if any(marker in line for marker in ['```', '###', 'NOTE:', 'TODO:', '- ', '* ']):
            continue
            
        # Look for C-style cast removals
        if line.startswith('-') and re.search(r'\([a-zA-Z_][\w\s*&]*\)\s*\w+', line):
            # Check if there's a corresponding static_cast addition
            for j in range(max(0, i-5), min(i+10, len(lines))):
                if lines[j].startswith('+') and 'static_cast<' in lines[j]:
                    return True
                    
        # Look for static_cast additions
        if line.startswith('+') and 'static_cast<' in line:
            # Make sure it's replacing a C-style cast
            for j in range(max(0, i-10), i):
                if lines[j].startswith('-') and '(' in lines[j] and ')' in lines[j]:
                    # Check if it looks like a C-style cast
                    if re.search(r'\([a-zA-Z_][\w\s*&]*\)\s*\w+', lines[j]):
                        return True
                        
    return False

def extract_static_cast_changes(input_file, output_file):
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
            file_has_casts = False
            current_file = line.split()[2]
            
            # Skip non-source files
            if not any(current_file.endswith(ext) for ext in ['.h', '.hpp', '.cpp', '.cc', '.c']):
                # Find next file
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Skip test files and examples
            if any(skip in current_file for skip in ['/tests/', '/examples/', '/test_', '_test.']):
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
                    if is_static_cast_change(lines, j):
                        file_has_casts = True
                j += 1
            
            # If this file has cast changes, include it
            if file_has_casts:
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
    
    # Count the changes
    cstyle_count = 0
    static_cast_count = 0
    
    for line in output_lines:
        if line.startswith('-') and re.search(r'\([a-zA-Z_][\w\s*&]*\)\s*\w+', line):
            cstyle_count += 1
        if line.startswith('+') and 'static_cast<' in line:
            static_cast_count += 1
    
    return cstyle_count, static_cast_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_04_static_cast.patch'
    
    print("Extracting C-style cast to static_cast conversions...")
    cstyle, static_cast, file_count = extract_static_cast_changes(input_file, output_file)
    
    print(f"\nExtracted static_cast changes:")
    print(f"  - C-style casts removed: {cstyle}")
    print(f"  - static_cast additions: {static_cast}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")