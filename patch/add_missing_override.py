#!/usr/bin/env python3
"""Add missing override keywords to virtual functions in controller classes"""

import re
import os

def add_override_to_virtual_functions(file_path):
    """Add override keyword to virtual functions that are missing it"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    modified = False
    new_lines = []
    in_class = False
    class_name = ""
    
    for i, line in enumerate(lines):
        original_line = line
        
        # Track if we're in a class definition
        class_match = re.match(r'\s*class\s+(\w+)', line)
        if class_match:
            in_class = True
            class_name = class_match.group(1)
        
        # Check for end of class
        if in_class and re.match(r'\s*};\s*$', line):
            in_class = False
        
        # Look for virtual functions without override
        if in_class:
            # Pattern: virtual <return_type> <func_name>(...) [const] ;
            # But NOT: virtual ~ClassName (destructors)
            # And NOT: = 0 (pure virtual)
            virtual_func = re.match(r'(\s*virtual\s+(?!~)[\w:&*<>\s]+\s+\w+\s*\([^)]*\)\s*(?:const\s*)?);(?!\s*override)', line)
            if virtual_func and '= 0' not in line and 'override' not in line:
                # Add override before the semicolon
                line = re.sub(r';(\s*)$', ' override;\\1', line)
                modified = True
        
        new_lines.append(line)
    
    if modified:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)
        return True
    return False

def main():
    # Controller files that likely need override keywords
    controller_files = [
        'selforg/controller/sox.h',
        'selforg/controller/sos.h',
        'selforg/controller/derpseudosensor.h',
        'selforg/controller/derbigcontroller.h',
        'selforg/controller/dercontroller.h',
        'selforg/controller/depcontroller.h',
        'selforg/controller/dinvert3channelcontroller.h',
        'selforg/controller/fourwheeled_controller.h',
        'selforg/controller/homeokinbase.h',
        'selforg/controller/invertmotornstep.h',
        'selforg/controller/invertmotorspace.h',
        'selforg/controller/invertmotorbigmodel.h',
        'selforg/controller/invertcontroller.h',
        'selforg/controller/multilayerffnn.h',
        'selforg/controller/onecontroller.h',
        'selforg/controller/pimax.h',
        'selforg/controller/replaycontroller.h',
        'selforg/controller/semox.h',
        'selforg/controller/sinecontroller.h',
        'selforg/controller/soml.h',
    ]
    
    fixed_count = 0
    
    print("Adding missing override keywords to controller classes...")
    for file_path in controller_files:
        if os.path.exists(file_path):
            if add_override_to_virtual_functions(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
        else:
            print(f"File not found: {file_path}")
    
    # Also check for more controller files
    for root, dirs, files in os.walk('selforg/controller'):
        for file in files:
            if file.endswith('.h'):
                file_path = os.path.join(root, file)
                if file_path not in controller_files:
                    if add_override_to_virtual_functions(file_path):
                        print(f"Fixed: {file_path}")
                        fixed_count += 1
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()