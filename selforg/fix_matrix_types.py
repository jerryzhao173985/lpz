#!/usr/bin/env python3
"""Fix matrix type declarations"""

import os
import re

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Fix matrix::const Matrix& -> const matrix::Matrix&
    content = re.sub(r'matrix::const\s+Matrix&', 'const matrix::Matrix&', content)
    
    # Fix const matrix::const Matrix& -> const matrix::Matrix& (in case of double const)
    content = re.sub(r'const\s+matrix::const\s+Matrix&', 'const matrix::Matrix&', content)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed {file_path}")
        return True
    return False

def main():
    base_dir = "/Users/jerry/lpzrobot_mac/selforg"
    
    # Fix specific files found with grep
    files_to_fix = [
        "controller/invertmotorbigmodel.cpp",
        "controller/invertmotorbigmodel.h",
        "controller/invertmotornstep.cpp",
        "controller/replaycontroller.h",
        "controller/old_stuff/dersimple.h"
    ]
    
    fixed_count = 0
    for filename in files_to_fix:
        file_path = os.path.join(base_dir, filename)
        if os.path.exists(file_path):
            if fix_file(file_path):
                fixed_count += 1
        else:
            print(f"File not found: {file_path}")
    
    print(f"\nFixed {fixed_count} files")

if __name__ == "__main__":
    main()