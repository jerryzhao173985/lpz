#!/usr/bin/env python3
"""
Clean static_cast patch by removing corrupted override additions.
Focus only on valid C-style to static_cast conversions.
"""

import re

def is_valid_static_cast(line):
    """Check if a line contains a valid static_cast conversion."""
    # Valid patterns:
    # - (type)value → static_cast<type>(value)
    # - Casting pointers, references, basic types
    
    # Skip lines with override after expressions (corrupted)
    if 'override' in line and '=' in line:
        return False
    
    # Look for static_cast patterns
    if 'static_cast<' in line:
        # Check if it's a complete cast with parentheses
        if re.search(r'static_cast<[^>]+>\([^)]*\)', line):
            return True
    
    return False

def is_corrupted_line(line):
    """Check if a line has corrupted changes."""
    corrupted_patterns = [
        r'override\s*;',  # override after semicolon
        r'=.*override',   # override after assignment
        r'explicit\s+explicit',  # duplicate explicit
        r'explicit\s+if',  # explicit before if
        r'explicit\s+port',  # explicit before port
        r'\)\s+override\s*;',  # override after closing paren
    ]
    
    for pattern in corrupted_patterns:
        if re.search(pattern, line):
            return True
    return False

def clean_static_cast_patch(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = set()
    skipped_count = 0
    
    while i < len(lines):
        line = lines[i]
        
        # Found a new file diff
        if line.startswith('diff --git'):
            file_start = i
            current_file = line.split()[2]
            
            # Skip non-source files
            if not any(current_file.endswith(ext) for ext in ['.h', '.hpp', '.cpp', '.cc']):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Collect the entire file diff
            j = i + 1
            file_lines = [line]
            file_has_valid_casts = False
            has_corrupted = False
            
            while j < len(lines) and not lines[j].startswith('diff --git'):
                file_lines.append(lines[j])
                
                # Check for corrupted lines
                if lines[j].startswith(('+', '-')) and is_corrupted_line(lines[j]):
                    has_corrupted = True
                    skipped_count += 1
                
                # Check for valid static_cast
                if lines[j].startswith(('+', '-')) and is_valid_static_cast(lines[j]):
                    file_has_valid_casts = True
                    
                j += 1
            
            # Include file only if it has valid casts and no corruption
            if file_has_valid_casts and not has_corrupted:
                # Filter hunks to include only those with valid static_cast
                filtered_lines = []
                k = 0
                
                while k < len(file_lines):
                    if file_lines[k].startswith(('diff --git', 'index ', '--- ', '+++ ')):
                        filtered_lines.append(file_lines[k])
                    elif file_lines[k].startswith('@@'):
                        # Check if this hunk contains valid static_cast
                        hunk_start = k
                        hunk_has_valid = False
                        hunk_has_corrupt = False
                        m = k + 1
                        
                        while m < len(file_lines) and not file_lines[m].startswith('@@'):
                            if file_lines[m].startswith(('+', '-')):
                                if is_corrupted_line(file_lines[m]):
                                    hunk_has_corrupt = True
                                elif is_valid_static_cast(file_lines[m]):
                                    hunk_has_valid = True
                            m += 1
                        
                        # Include hunk only if valid and not corrupt
                        if hunk_has_valid and not hunk_has_corrupt:
                            for n in range(hunk_start, m):
                                filtered_lines.append(file_lines[n])
                        k = m - 1
                    k += 1
                
                if len(filtered_lines) > 4:  # Has content beyond headers
                    output_lines.extend(filtered_lines)
                    files_with_changes.add(current_file)
            
            i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count valid changes
    cast_count = sum(1 for line in output_lines if 'static_cast<' in line and line.startswith(('+', '-')))
    
    return cast_count, len(files_with_changes), skipped_count

if __name__ == "__main__":
    input_file = 'patch_04_static_cast.patch'
    output_file = 'patch_04_static_cast_clean.patch'
    
    print("Cleaning static_cast patch...")
    cast_count, file_count, skipped = clean_static_cast_patch(input_file, output_file)
    
    print(f"\nCleaned static_cast patch:")
    print(f"  - Valid static_cast changes: {cast_count}")
    print(f"  - Files with clean changes: {file_count}")
    print(f"  - Skipped corrupted lines: {skipped}")
    print(f"  - Output written to: {output_file}")