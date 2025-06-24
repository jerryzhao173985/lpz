#!/usr/bin/env python3
"""
Fix C++ warnings in opende component
"""
import os
import re
import sys

def fix_va_end_missing(file_path):
    """Fix missing va_end() calls"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Find va_start without matching va_end
    lines = content.split('\n')
    modified = False
    new_lines = []
    
    for i, line in enumerate(lines):
        new_lines.append(line)
        
        # Check if this line contains va_start
        if 'va_start' in line and 'va_end' not in line:
            # Look for the end of the function (closing brace at start of line)
            func_level = 0
            in_func = False
            has_va_end = False
            
            # First find the opening brace of the function
            for j in range(i, min(i + 50, len(lines))):
                if '{' in lines[j]:
                    in_func = True
                    func_level += lines[j].count('{')
                    func_level -= lines[j].count('}')
                elif in_func:
                    func_level += lines[j].count('{')
                    func_level -= lines[j].count('}')
                
                if 'va_end' in lines[j]:
                    has_va_end = True
                    break
                    
                if in_func and func_level == 0:
                    # End of function found
                    if not has_va_end and j > i:
                        # Extract va_list variable name
                        va_match = re.search(r'va_start\s*\(\s*([^,]+)\s*,', line)
                        if va_match:
                            va_var = va_match.group(1).strip()
                            # Insert va_end before the closing brace
                            insert_idx = len(new_lines) + (j - i) - 1
                            indent = '  ' if lines[j-1].startswith('  ') else ''
                            # We'll mark this for later insertion
                            new_lines.append(f"__VA_END_MARKER__{va_var}__{insert_idx}")
                            modified = True
                    break
    
    # Now process the markers
    final_lines = []
    for line in new_lines:
        if line.startswith("__VA_END_MARKER__"):
            parts = line.split("__")
            va_var = parts[2]
            insert_idx = int(parts[3])
            # Don't add the marker line
            continue
        final_lines.append(line)
        
        # Check if we need to insert va_end
        if len(final_lines) == insert_idx:
            final_lines.insert(-1, f"  va_end({va_var});")
    
    if modified:
        with open(file_path, 'w') as f:
            f.write('\n'.join(final_lines))
        return True
    return False

def fix_uninitialized_members(file_path):
    """Fix uninitialized member variables"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Common patterns for constructors that need member initialization
    patterns = [
        # Array base constructor
        (r'(dArrayBase::dArrayBase\s*\([^)]*\)\s*:\s*)([^{]+)\s*{',
         r'\1\2, _size(0), _anum(0), _data(NULL) {'),
        
        # dxJointLMotor constructor - need to find actual constructor
        (r'(dxJointLMotor::dxJointLMotor\s*\([^)]*\))\s*:\s*([^{]+)\s*{',
         r'\1 : \2 { memset(rel, 0, sizeof(rel)); memset(limot, 0, sizeof(limot));'),
        
        # dxJointPlane2D constructor
        (r'(dxJointPlane2D::dxJointPlane2D\s*\([^)]*\))\s*:\s*([^{]+)\s*{',
         r'\1 : \2, row_motor_x(0), row_motor_y(0), row_motor_angle(0) {'),
    ]
    
    modified = False
    for pattern, replacement in patterns:
        if re.search(pattern, content):
            content = re.sub(pattern, replacement, content)
            modified = True
    
    if modified:
        with open(file_path, 'w') as f:
            f.write(content)
        return True
    return False

def fix_alloca_calls(file_path):
    """Replace alloca with stack arrays or malloc"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Replace alloca with std::vector or malloc
    # Pattern: type* var = (type*)alloca(size)
    alloca_pattern = r'(\w+)\s*\*\s*(\w+)\s*=\s*\([^)]+\)\s*alloca\s*\(([^)]+)\)'
    
    modified = False
    
    def replace_alloca(match):
        type_name = match.group(1)
        var_name = match.group(2)
        size_expr = match.group(3)
        
        # If size is simple sizeof expression, use stack array
        if 'sizeof' in size_expr and '*' in size_expr:
            # Extract the count
            count_match = re.search(r'(\w+)\s*\*\s*sizeof', size_expr)
            if count_match:
                count = count_match.group(1)
                return f"std::vector<{type_name}> {var_name}_vec({count}); {type_name}* {var_name} = {var_name}_vec.data()"
        
        # Otherwise use malloc with free at end
        return f"{type_name}* {var_name} = ({type_name}*)malloc({size_expr})"
    
    if 'alloca' in content:
        # First add vector include if needed
        if 'std::vector' in content and '#include <vector>' not in content:
            # Add after other includes
            include_pos = content.find('#include')
            if include_pos != -1:
                end_of_line = content.find('\n', include_pos)
                content = content[:end_of_line+1] + '#include <vector>\n' + content[end_of_line+1:]
        
        content = re.sub(alloca_pattern, replace_alloca, content)
        modified = True
    
    if modified:
        with open(file_path, 'w') as f:
            f.write(content)
        return True
    return False

def process_file(file_path):
    """Process a single file for all warning fixes"""
    if not os.path.exists(file_path):
        return False
    
    print(f"Processing {file_path}")
    
    fixed = False
    
    # Fix different types of warnings
    if file_path.endswith('.cpp') or file_path.endswith('.c'):
        if fix_va_end_missing(file_path):
            print(f"  Fixed va_end issues")
            fixed = True
            
        if fix_alloca_calls(file_path):
            print(f"  Fixed alloca calls")
            fixed = True
    
    if file_path.endswith('.h') or file_path.endswith('.cpp'):
        if fix_uninitialized_members(file_path):
            print(f"  Fixed uninitialized members")
            fixed = True
    
    return fixed

def main():
    # Files with specific warnings
    files_to_fix = [
        'opende/ode/src/error.cpp',  # va_end missing
        'opende/ode/src/array.h',  # uninitialized members
        'opende/ode/src/joints/lmotor.cpp',  # uninitialized members
        'opende/ode/src/joints/plane2d.cpp',  # uninitialized members
        'opende/ode/src/collision_space.cpp',  # alloca calls
        'opende/ode/src/lcp.cpp',  # alloca calls
        'opende/ode/src/fastldlt.c',  # null pointer
    ]
    
    os.chdir('/Users/jerry/lpzrobot_mac')
    
    fixed_count = 0
    for file_path in files_to_fix:
        if process_file(file_path):
            fixed_count += 1
    
    print(f"\nFixed {fixed_count} files")

if __name__ == '__main__':
    main()