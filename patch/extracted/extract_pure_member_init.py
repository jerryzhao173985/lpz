#!/usr/bin/env python3
"""
Extract ONLY pure member initialization changes.
Focus on class member declarations with initialization.
"""

import re

def is_pure_member_init(line):
    """Check if a line is a pure member initialization in class declaration."""
    # Patterns for member initialization:
    # 1. int m_var = 0;
    # 2. double m_value{0.0};
    # 3. bool flag = false;
    # 4. Type* ptr = nullptr;
    
    # Must have = or {} for initialization
    if not ('=' in line or ('{' in line and '}' in line)):
        return False
        
    # Skip function calls, assignments in code
    if any(keyword in line for keyword in ['if', 'for', 'while', 'new', 'return', 'explicit']):
        return False
    
    # Look for member variable patterns (common prefixes: m_, _, or direct)
    member_patterns = [
        r'^\s*(int|double|float|bool|size_t|unsigned|uint\d+_t)\s+\w+\s*=\s*[^;]+;',
        r'^\s*(int|double|float|bool|size_t|unsigned|uint\d+_t)\s+\w+\s*\{[^}]*\};',
        r'^\s*\w+\*\s+\w+\s*=\s*nullptr\s*;',  # pointer initialization
        r'^\s*(m_|_)\w+\s*=\s*[^;]+;',  # member with prefix
    ]
    
    clean_line = line.strip()
    if clean_line.startswith(('+', '-')):
        clean_line = clean_line[1:].strip()
    
    for pattern in member_patterns:
        if re.match(pattern, clean_line):
            return True
            
    return False

def is_in_class_context(lines, index):
    """Check if current line is inside a class/struct definition."""
    # Look backwards for class/struct declaration
    for i in range(max(0, index - 50), index):
        if re.search(r'^\s*(class|struct)\s+\w+', lines[i]):
            # Check we haven't left the class
            brace_count = 0
            for j in range(i, index):
                brace_count += lines[j].count('{') - lines[j].count('}')
            return brace_count > 0
    return False

def extract_pure_member_init(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = set()
    
    while i < len(lines):
        line = lines[i]
        
        # Found a new file diff
        if line.startswith('diff --git'):
            file_start = i
            current_file = line.split()[2]
            
            # Only process header files (where member declarations are)
            if not current_file.endswith(('.h', '.hpp')):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Collect the entire file diff
            j = i + 1
            file_lines = [line]
            file_has_init = False
            
            while j < len(lines) and not lines[j].startswith('diff --git'):
                file_lines.append(lines[j])
                
                # Check if this is a member initialization
                if lines[j].startswith(('+', '-')) and is_pure_member_init(lines[j]):
                    # Verify it's in class context
                    if is_in_class_context(file_lines, len(file_lines) - 1):
                        file_has_init = True
                    
                j += 1
            
            # If this file has member init changes, include it
            if file_has_init:
                # Filter to include only member init hunks
                filtered_lines = []
                k = 0
                
                while k < len(file_lines):
                    if file_lines[k].startswith(('diff --git', 'index ', '--- ', '+++ ')):
                        filtered_lines.append(file_lines[k])
                    elif file_lines[k].startswith('@@'):
                        # Check if this hunk contains member init
                        hunk_start = k
                        hunk_has_init = False
                        m = k + 1
                        
                        while m < len(file_lines) and not file_lines[m].startswith('@@'):
                            if file_lines[m].startswith(('+', '-')) and is_pure_member_init(file_lines[m]):
                                hunk_has_init = True
                            m += 1
                        
                        # Include hunk if it has member init
                        if hunk_has_init:
                            for n in range(hunk_start, m):
                                filtered_lines.append(file_lines[n])
                        k = m - 1
                    k += 1
                
                if len(filtered_lines) > 4:
                    output_lines.extend(filtered_lines)
                    files_with_changes.add(current_file)
            
            i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count the changes
    init_count = sum(1 for line in output_lines if line.startswith(('+', '-')) and is_pure_member_init(line))
    
    return init_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch_06_member_init_only.patch'
    output_file = 'patch_06_pure_member_init.patch'
    
    print("Extracting pure member initialization changes...")
    init_count, file_count = extract_pure_member_init(input_file, output_file)
    
    print(f"\nExtracted pure member initialization:")
    print(f"  - Member init changes: {init_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")