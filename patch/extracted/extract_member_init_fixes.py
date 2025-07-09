#!/usr/bin/env python3
"""
Extract member initialization fixes from the patch.
Focus on constructor initialization lists that fix uninitialized members.
"""

import re

def is_member_init_change(lines, start_idx):
    """Check if a hunk contains member initialization fixes."""
    # Look for constructor with initialization list pattern
    for i in range(start_idx, min(start_idx + 50, len(lines))):
        line = lines[i]
        
        # Pattern for constructor with initialization list
        if re.search(r'^\+.*\w+::\w+\([^)]*\)\s*:', line):
            # Check if initialization list follows
            j = i + 1
            init_found = False
            while j < len(lines) and j < i + 20:
                if lines[j].startswith('+') and re.search(r'^\+\s*[:,]\s*\w+\([^)]*\)', lines[j]):
                    init_found = True
                if lines[j].strip().endswith('{'):
                    break
                j += 1
            if init_found:
                return True
        
        # Also look for in-class member initializers
        if line.startswith('+') and re.search(r'=\s*\{.*\}|=\s*\d+|=\s*nullptr|=\s*false|=\s*true', line):
            # Check if it's a member variable declaration
            if re.search(r'^\+\s*(private|protected|public)?\s*\w+.*\w+\s*=', line):
                return True
    
    return False

def extract_member_init_changes(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    output_lines = []
    i = 0
    files_with_changes = []
    init_examples = []
    
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('diff --git'):
            file_start = i
            current_file = line.split()[2]
            file_has_init = False
            
            # Focus on C++ source files
            if not current_file.endswith(('.cpp', '.h', '.hpp', '.cc')):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Skip documentation
            if any(x in current_file for x in ['docs/', 'README', 'CHANGELOG', '.md', 'examples/']):
                j = i + 1
                while j < len(lines) and not lines[j].startswith('diff --git'):
                    j += 1
                i = j
                continue
            
            # Collect the entire file diff
            j = i + 1
            while j < len(lines) and not lines[j].startswith('diff --git'):
                if lines[j].startswith('@@'):
                    hunk_start = j
                    if is_member_init_change(lines, j):
                        file_has_init = True
                        
                        # Collect example
                        example_lines = []
                        for k in range(hunk_start, min(hunk_start + 30, len(lines))):
                            if lines[k].startswith('diff --git'):
                                break
                            if lines[k].startswith(('+', '-', '@@')):
                                example_lines.append(lines[k].rstrip())
                        
                        if len(init_examples) < 5:  # Collect up to 5 examples
                            init_examples.append({
                                'file': current_file,
                                'lines': example_lines
                            })
                j += 1
            
            if file_has_init:
                for k in range(file_start, j):
                    output_lines.append(lines[k])
                files_with_changes.append(current_file)
            
            i = j
        else:
            i += 1
    
    # Write the output
    with open(output_file, 'w') as f:
        f.writelines(output_lines)
    
    # Write examples for review
    with open('patch/extracted/member_init_examples.txt', 'w') as f:
        f.write("MEMBER INITIALIZATION EXAMPLES\n")
        f.write("=" * 50 + "\n\n")
        
        for example in init_examples:
            f.write(f"\nFile: {example['file']}\n")
            f.write("-" * 40 + "\n")
            for line in example['lines'][:15]:  # Show first 15 lines
                f.write(line + "\n")
            f.write("\n")
    
    # Count initialization patterns
    init_count = 0
    for line in output_lines:
        if line.startswith('+') and (': ' in line or ' = ' in line):
            if re.search(r'[:,]\s*\w+\([^)]*\)|=\s*(?:\{.*\}|\d+|nullptr|false|true)', line):
                init_count += 1
    
    return init_count, len(files_with_changes)

if __name__ == "__main__":
    input_file = 'patch/extracted/source_changes.patch'
    output_file = 'patch/extracted/patch_04_member_initialization.patch'
    
    print("Extracting member initialization fixes...")
    init_count, file_count = extract_member_init_changes(input_file, output_file)
    
    print(f"Extracted member initialization changes:")
    print(f"  - Initialization additions: {init_count}")
    print(f"  - Files affected: {file_count}")
    print(f"  - Output written to: {output_file}")
    print(f"  - Examples written to: member_init_examples.txt")