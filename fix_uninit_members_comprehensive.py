#!/usr/bin/env python3
"""
Fix uninitialized member variables in C++ code.
This script adds default member initializers to class/struct members.
"""

import os
import re
import sys

def fix_uninit_members_in_file(filepath):
    """Fix uninitialized member variables in a single file."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return 0
    
    original_content = content
    changes = 0
    
    # Pattern to find class or struct definitions
    class_pattern = re.compile(
        r'(class|struct)\s+(\w+)(?:\s*:\s*(?:public|protected|private)?\s*[\w:]+)?[^{]*\{([^}]+)\}',
        re.MULTILINE | re.DOTALL
    )
    
    def process_class_body(match):
        nonlocal changes
        class_type = match.group(1)
        class_name = match.group(2)
        body = match.group(3)
        
        # Skip if it's a forward declaration or template specialization
        if ';' not in body:
            return match.group(0)
        
        # Find member variable declarations
        lines = body.split('\n')
        new_lines = []
        
        for line in lines:
            # Skip empty lines, comments, methods, and access specifiers
            stripped = line.strip()
            if (not stripped or 
                stripped.startswith('//') or 
                stripped.startswith('/*') or
                stripped.startswith('*') or
                stripped.endswith('*/') or
                stripped in ['public:', 'protected:', 'private:'] or
                '(' in stripped or  # Skip methods
                '{' in stripped or  # Skip inline methods
                'typedef' in stripped or
                'using' in stripped or
                'static' in stripped or  # Skip static members
                'const static' in stripped or
                'friend' in stripped or
                'template' in stripped):
                new_lines.append(line)
                continue
            
            # Check if this is a member variable declaration
            # Pattern: type name; or type name[size];
            member_pattern = re.compile(
                r'^(\s*)((?:const\s+)?(?:unsigned\s+)?(?:mutable\s+)?)'
                r'(bool|int|short|long|float|double|char|size_t|unsigned|'
                r'sensor|motor|matrix_t|BYTE|WORD|DWORD|dReal|'
                r'paramval|paramint|parambool)\s+'
                r'(\w+(?:\[[^\]]*\])?)\s*;(.*)$'
            )
            
            match = member_pattern.match(line)
            if match:
                indent = match.group(1)
                modifiers = match.group(2)
                var_type = match.group(3)
                var_name = match.group(4)
                rest = match.group(5)
                
                # Check if already initialized
                if '=' in var_name:
                    new_lines.append(line)
                    continue
                
                # Determine appropriate default value
                if var_type == 'bool' or var_type == 'parambool':
                    default = 'false'
                elif var_type in ['int', 'short', 'long', 'size_t', 'unsigned', 
                                 'paramint', 'BYTE', 'WORD', 'DWORD']:
                    default = '0'
                elif var_type in ['float', 'double', 'paramval', 'sensor', 'motor', 
                                 'matrix_t', 'dReal']:
                    default = '0.0'
                elif var_type == 'char':
                    default = "'\\0'"
                else:
                    # Unknown type, skip
                    new_lines.append(line)
                    continue
                
                # Handle arrays
                if '[' in var_name:
                    # Arrays can't have default member initializers in C++11
                    # They need to be initialized in constructor
                    new_lines.append(line)
                    continue
                
                # Add default initializer
                new_line = f"{indent}{modifiers}{var_type} {var_name} = {default};{rest}"
                new_lines.append(new_line)
                changes += 1
                print(f"  Initialized {var_name} in {class_name}")
            else:
                # Check for pointer declarations
                pointer_pattern = re.compile(
                    r'^(\s*)((?:const\s+)?(?:unsigned\s+)?(?:mutable\s+)?)'
                    r'([\w:]+)\s*\*+\s*(\w+)\s*;(.*)$'
                )
                
                ptr_match = pointer_pattern.match(line)
                if ptr_match:
                    indent = ptr_match.group(1)
                    modifiers = ptr_match.group(2)
                    ptr_type = ptr_match.group(3)
                    ptr_name = ptr_match.group(4)
                    rest = ptr_match.group(5)
                    
                    # Check if already initialized
                    if '=' not in line:
                        new_line = f"{indent}{modifiers}{ptr_type}* {ptr_name} = nullptr;{rest}"
                        new_lines.append(new_line)
                        changes += 1
                        print(f"  Initialized pointer {ptr_name} in {class_name}")
                    else:
                        new_lines.append(line)
                else:
                    new_lines.append(line)
        
        if changes > 0:
            new_body = '\n'.join(new_lines)
            return f"{class_type} {class_name}{match.group(0)[match.group(0).find('{'):][:1]}{new_body}}}"
        else:
            return match.group(0)
    
    # Apply fixes to all classes/structs
    content = class_pattern.sub(process_class_body, content)
    
    if content != original_content:
        try:
            with open(filepath, 'w') as f:
                f.write(content)
            if changes > 0:
                print(f"Fixed {changes} uninitialized members in {filepath}")
        except Exception as e:
            print(f"Error writing {filepath}: {e}")
            return 0
    
    return changes

def main():
    if len(sys.argv) > 1:
        # Process specific files
        total_changes = 0
        for filepath in sys.argv[1:]:
            if os.path.isfile(filepath) and (filepath.endswith('.h') or filepath.endswith('.cpp')):
                total_changes += fix_uninit_members_in_file(filepath)
        print(f"\nTotal uninitialized members fixed: {total_changes}")
    else:
        # Process all .h and .cpp files in selforg and ode_robots
        total_changes = 0
        dirs_to_process = ['selforg', 'ode_robots']
        
        for dir_name in dirs_to_process:
            if os.path.exists(dir_name):
                for root, dirs, files in os.walk(dir_name):
                    # Skip .svn directories
                    if '.svn' in root:
                        continue
                    for file in files:
                        if file.endswith(('.h', '.cpp')):
                            filepath = os.path.join(root, file)
                            total_changes += fix_uninit_members_in_file(filepath)
        
        print(f"\nTotal uninitialized members fixed: {total_changes}")

if __name__ == '__main__':
    main()