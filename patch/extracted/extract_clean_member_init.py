#!/usr/bin/env python3
"""
Extract ONLY member initialization fixes, excluding override keyword changes.
Focus on actual initialization improvements.
"""

import re

def is_real_member_init(line):
    """Check if line contains real member initialization, not override fixes."""
    # Skip lines with override keyword
    if 'override' in line:
        return False
    
    # Patterns for member initialization
    init_patterns = [
        r':\s*\w+\([^)]*\)',  # Constructor init list
        r':\s*\w+\{[^}]*\}',  # Brace initialization
        r'=\s*nullptr\s*;',   # nullptr initialization
        r'=\s*false\s*;',     # bool initialization
        r'=\s*true\s*;',      # bool initialization
        r'=\s*0\s*;',         # zero initialization
        r'=\s*\d+\s*;',       # numeric initialization
        r'=\s*\{[^}]*\}\s*;', # aggregate initialization
    ]
    
    for pattern in init_patterns:
        if re.search(pattern, line):
            return True
    
    return False

def extract_clean_member_init(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = []
    
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('diff --git'):
            file_start = i
            current_file = line.split()[2]
            file_has_init = False
            
            # Skip non-C++ files
            if not current_file.endswith(('.cpp', '.h', '.hpp', '.cc')):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Skip docs and examples
            if any(x in current_file for x in ['docs/', '.md', 'examples/', 'test']):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Check if file has real member init changes
            j = i + 1
            temp_lines = []
            while j < len(lines) and not lines[j].startswith('diff --git'):
                temp_lines.append(lines[j])
                
                # Check for real initialization changes
                if lines[j].startswith('+') and is_real_member_init(lines[j]):
                    # Verify it's not just removing override
                    context_has_init = False
                    for k in range(max(0, j-5), min(j+5, len(lines))):
                        if lines[k].startswith('+') and is_real_member_init(lines[k]):
                            context_has_init = True
                            break
                    
                    if context_has_init:
                        file_has_init = True
                
                j += 1
            
            # Only include files with real initialization changes
            if file_has_init:
                # Filter out hunks that only have override changes
                output_lines.append(lines[file_start])  # diff --git line
                
                hunk_buffer = []
                in_hunk = False
                hunk_has_init = False
                
                for tline in temp_lines:
                    if tline.startswith('@@'):
                        # Process previous hunk
                        if in_hunk and hunk_has_init:
                            output_lines.extend(hunk_buffer)
                        
                        # Start new hunk
                        hunk_buffer = [tline]
                        in_hunk = True
                        hunk_has_init = False
                    else:
                        hunk_buffer.append(tline)
                        if tline.startswith('+') and is_real_member_init(tline):
                            hunk_has_init = True
                
                # Process last hunk
                if in_hunk and hunk_has_init:
                    output_lines.extend(hunk_buffer)
                
                files_with_changes.append(current_file)
            
            i = j
        else:
            i += 1
    
    # Write output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Count real initializations
    init_count = 0
    for line in output_lines:
        if line.startswith('+') and is_real_member_init(line):
            init_count += 1
    
    return init_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_04_clean_member_init.patch'
    
    print("Extracting clean member initialization fixes...")
    init_count, file_count = extract_clean_member_init(input_file, output_file)
    
    print(f"Extracted clean member initialization changes:")
    print(f"  - Real initialization additions: {init_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")
    
    if file_count == 0:
        print("\nNo clean member initialization changes found.")
        print("Most changes appear to be override keyword fixes.")