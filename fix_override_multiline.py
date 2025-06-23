#!/usr/bin/env python3
"""Add override keywords to functions that span multiple lines."""

import re
import os

def add_override_to_function(content, function_name, line_num):
    """Add override keyword to a function declaration."""
    lines = content.split('\n')
    
    # Start from the specified line
    start_idx = line_num - 1
    if start_idx >= len(lines):
        return content, False
    
    # Find the complete function declaration (may span multiple lines)
    function_text = ""
    paren_count = 0
    found_start = False
    end_idx = start_idx
    
    # Search for function starting from the line
    for i in range(start_idx, min(start_idx + 10, len(lines))):
        line = lines[i]
        
        # Check if this line contains the function name
        if function_name in line and not found_start:
            found_start = True
        
        if found_start:
            function_text += line + "\n"
            paren_count += line.count('(') - line.count(')')
            
            # Check if we've found the end of the function declaration
            if paren_count > 0 and ')' in line:
                # Continue until we balance parentheses
                if paren_count == 0:
                    end_idx = i
                    break
            elif paren_count == 0 and (';' in line or '{' in line):
                end_idx = i
                break
    
    if not found_start:
        return content, False
    
    # Check if override already exists
    if 'override' in function_text:
        return content, False
    
    # Add override before the final semicolon or opening brace
    # Handle the complete function declaration
    modified_lines = lines[:start_idx]
    
    # Reconstruct the function with override
    func_lines = function_text.strip().split('\n')
    last_line = func_lines[-1]
    
    # Find where to insert override
    if ';' in last_line:
        insert_pos = last_line.rfind(';')
        new_last_line = last_line[:insert_pos] + ' override;' + last_line[insert_pos+1:]
    elif '{' in last_line:
        insert_pos = last_line.rfind('{')
        new_last_line = last_line[:insert_pos] + ' override {' + last_line[insert_pos+1:]
    else:
        # Might be on the next line
        return content, False
    
    func_lines[-1] = new_last_line
    
    # Add modified function lines
    for i, line in enumerate(func_lines):
        if i < len(func_lines) - 1:
            modified_lines.append(lines[start_idx + i])
        else:
            modified_lines.append(line.rstrip())
    
    # Add remaining lines
    modified_lines.extend(lines[end_idx + 1:])
    
    return '\n'.join(modified_lines), True

def process_file(filepath, warnings):
    """Process a single file to add override keywords."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    # Create backup
    backup_path = filepath + '.bak.override3'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    modified = False
    for function_name, line_num in warnings:
        content, changed = add_override_to_function(content, function_name, line_num)
        if changed:
            modified = True
            print(f"Added override to {function_name} at line {line_num} in {filepath}")
    
    if modified:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    
    return False

def main():
    # Manually specify warnings for key files
    file_warnings = {
        '/Users/jerry/lpzrobot_mac/selforg/controller/abstractcontrolleradapter.h': [
            ('step', 99),
            ('stepNoLearning', 107),
            ('store', 142),
            ('restore', 147)
        ],
        '/Users/jerry/lpzrobot_mac/selforg/controller/abstractiafcontroller.h': [
            ('step', 101),
            ('stepNoLearning', 103),
            ('store', 107),
            ('restore', 109),
            ('notifyOnChange', 112)
        ],
        '/Users/jerry/lpzrobot_mac/selforg/controller/abstractmulticontroller.h': [
            ('step', 86),
            ('stepNoLearning', 92)
        ],
        '/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.h': [
            ('step', 69),
            ('stepNoLearning', 72),
            ('store', 78),
            ('restore', 80),
            ('getInternalParamNames', 83),
            ('getInternalParams', 84),
            ('getStructuralLayers', 85),
            ('getStructuralConnections', 86),
            ('notifyOnChange', 89)
        ],
        '/Users/jerry/lpzrobot_mac/selforg/controller/invertmotornstep.h': [
            ('step', 71),
            ('stepNoLearning', 74),
            ('store', 79),
            ('restore', 81),
            ('getStructuralLayers', 84),
            ('getStructuralConnections', 85)
        ],
        '/Users/jerry/lpzrobot_mac/selforg/controller/invertmotorspace.h': [
            ('step', 51),
            ('stepNoLearning', 54),
            ('store', 59),
            ('restore', 61),
            ('getStructuralLayers', 64),
            ('getStructuralConnections', 65)
        ],
        '/Users/jerry/lpzrobot_mac/selforg/controller/invertnchannelcontroller.h': [
            ('getName', 47),
            ('step', 55),
            ('stepNoLearning', 59),
            ('store', 65),
            ('restore', 67)
        ]
    }
    
    modified_count = 0
    for filepath, warnings in file_warnings.items():
        if process_file(filepath, warnings):
            modified_count += 1
    
    print(f"\nModified {modified_count} files")

if __name__ == '__main__':
    main()