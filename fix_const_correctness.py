#!/usr/bin/env python3
"""Fix const correctness issues in the codebase."""

import re
import os

def fix_const_issues_in_file(filepath, issues):
    """Fix const correctness issues in a specific file."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    # Create backup
    backup_path = filepath + '.bak.const'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    modified = False
    
    for line_num, var_name, fix_type in issues:
        lines = content.split('\n')
        if line_num <= len(lines):
            line_idx = line_num - 1
            line = lines[line_idx]
            
            if fix_type == 'constVariableReference':
                # Change auto& to const auto&
                if f'auto& {var_name}' in line:
                    lines[line_idx] = line.replace(f'auto& {var_name}', f'const auto& {var_name}')
                    modified = True
                    print(f"  Line {line_num}: Made {var_name} const reference")
                # Change Type& to const Type&
                elif f'& {var_name}' in line and 'const' not in line:
                    # Find the type before &
                    match = re.search(rf'(\w+)\s*&\s*{var_name}', line)
                    if match:
                        type_name = match.group(1)
                        lines[line_idx] = line.replace(f'{type_name}& {var_name}', f'const {type_name}& {var_name}')
                        modified = True
                        print(f"  Line {line_num}: Made {var_name} const reference")
            
            elif fix_type == 'constVariablePointer':
                # Change Type* to const Type*
                if f'* {var_name}' in line and 'const' not in line:
                    match = re.search(rf'(\w+)\s*\*\s*{var_name}', line)
                    if match:
                        type_name = match.group(1)
                        lines[line_idx] = line.replace(f'{type_name}* {var_name}', f'const {type_name}* {var_name}')
                        modified = True
                        print(f"  Line {line_num}: Made {var_name} pointer to const")
            
            elif fix_type == 'constParameterReference':
                # Fix function parameters
                if var_name in line and '&' in line and 'const' not in line:
                    # Try to find the parameter in function declaration
                    match = re.search(rf'(\w+)\s*&\s*{var_name}', line)
                    if match:
                        type_name = match.group(1)
                        lines[line_idx] = line.replace(f'{type_name}& {var_name}', f'const {type_name}& {var_name}')
                        modified = True
                        print(f"  Line {line_num}: Made parameter {var_name} const reference")
    
    if modified:
        content = '\n'.join(lines)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    
    return False

def main():
    # Define specific const issues to fix
    const_fixes = {
        '/Users/jerry/lpzrobot_mac/selforg/controller/abstractcontroller.cpp': [
            (50, 'i', 'constVariableReference'),
            (65, 'i', 'constVariableReference')
        ],
        '/Users/jerry/lpzrobot_mac/selforg/matrix/matrix.cpp': [
            (891, 'p1', 'constVariablePointer'),
            (892, 'p2', 'constVariablePointer')
        ],
        '/Users/jerry/lpzrobot_mac/selforg/matrix/matrixutils.cpp': [
            (202, 'eigenvalues', 'constParameterReference'),
            (203, 'eigenvectors', 'constParameterReference'),
            (208, 'real', 'constParameterReference'),
            (208, 'imag', 'constParameterReference'),
            (213, 'vals_real', 'constParameterReference'),
            (213, 'vals_imag', 'constParameterReference'),
            (214, 'vecs_real', 'constParameterReference'),
            (214, 'vecs_imag', 'constParameterReference')
        ]
    }
    
    for filepath, issues in const_fixes.items():
        print(f"\nProcessing {filepath}:")
        if fix_const_issues_in_file(filepath, issues):
            print("  Fixed const correctness issues")
        else:
            print("  No changes made")

if __name__ == '__main__':
    main()