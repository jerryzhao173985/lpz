#!/usr/bin/env python3
"""Fix nullptr used in numeric contexts where 0 should be used"""

import re
import os

def fix_nullptr_numeric(file_path):
    """Fix nullptr incorrectly used in numeric contexts"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    modified = False
    new_lines = []
    
    for i, line in enumerate(lines):
        original_line = line
        
        # Fix: for(int i = nullptr; ...) -> for(int i = 0; ...)
        line = re.sub(r'for\s*\(\s*int\s+(\w+)\s*=\s*nullptr\s*;', r'for(int \1 = 0;', line)
        
        # Fix: for(size_t i = nullptr; ...) -> for(size_t i = 0; ...)
        line = re.sub(r'for\s*\(\s*size_t\s+(\w+)\s*=\s*nullptr\s*;', r'for(size_t \1 = 0;', line)
        
        # Fix: for(unsigned i = nullptr; ...) -> for(unsigned i = 0; ...)
        line = re.sub(r'for\s*\(\s*unsigned\s+(\w+)\s*=\s*nullptr\s*;', r'for(unsigned \1 = 0;', line)
        
        # Fix: numeric variable = nullptr -> = 0
        line = re.sub(r'(int|size_t|unsigned|long|short|double|float)\s+(\w+)\s*=\s*nullptr\s*;', r'\1 \2 = 0;', line)
        
        # Fix array index comparisons: [i] != nullptr -> specific handling needed
        # Skip these for now as they need context
        
        if line != original_line:
            modified = True
            
        new_lines.append(line)
    
    if modified:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.writelines(new_lines)
        return True
    return False

def main():
    # Specific files with numeric nullptr issues from the extraction
    problem_files = [
        ('main.cpp', 114454),  # for (int i= nullptr; i < 0/*2*/; ++i)
        ('ode_robots/obstacles/tracksections/degreesegment.cpp', 92917),  # for loop
        ('ode_robots/simulations/humanoid/main.cpp', 96655),  # for loop
        ('ode_robots/simulations/manipulation/main.cpp', 98102),  # for loop  
        ('ode_robots/simulations/schlangeservo/main.cpp', 98866),  # for loop
        ('ode_robots/simulations/sphericalrobot/main.cpp', 99549),  # for loop
        ('real_robots/simulations/src/main.cpp', 149966),  # for loop
        ('selforg/controller/use_java_controller.cpp', 166509),  # for loop
    ]
    
    fixed_count = 0
    
    # First pass: Fix known numeric nullptr issues
    for file_path, line_num in problem_files:
        if os.path.exists(file_path):
            if fix_nullptr_numeric(file_path):
                print(f"Fixed numeric nullptr in: {file_path} (around line {line_num})")
                fixed_count += 1
        else:
            # Try with different paths
            alt_paths = [
                f"ode_robots/simulations/{os.path.basename(os.path.dirname(file_path))}/main.cpp",
                file_path
            ]
            for alt_path in alt_paths:
                if os.path.exists(alt_path):
                    if fix_nullptr_numeric(alt_path):
                        print(f"Fixed numeric nullptr in: {alt_path}")
                        fixed_count += 1
                    break
    
    print(f"\nTotal files fixed: {fixed_count}")
    
    # Note: Not touching pointer comparisons with nullptr as those are valid

if __name__ == "__main__":
    main()