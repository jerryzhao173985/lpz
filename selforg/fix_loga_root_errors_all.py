#!/usr/bin/env python3
"""Fix all logaE/rootE errors in controller files"""

import os
import re

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Fix logaE and rootE patterns
    content = re.sub(r'\(const logaE& (\d+)\) != 0', r'(logaE >= \1)', content)
    content = re.sub(r'\(const rootE& (\d+)\) != 0', r'(rootE >= \1)', content)
    
    # Also remove any override; that might be attached
    content = re.sub(r'\s+override;', ';', content)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed {file_path}")
        return True
    return False

def main():
    controller_dir = "/Users/jerry/lpzrobot_mac/selforg/controller"
    
    # Fix specific files found with grep
    files_to_fix = [
        "invertmotorbigmodel.cpp",
        "invertmotorspace.cpp", 
        "semox.cpp"
    ]
    
    fixed_count = 0
    for filename in files_to_fix:
        file_path = os.path.join(controller_dir, filename)
        if os.path.exists(file_path):
            if fix_file(file_path):
                fixed_count += 1
        else:
            print(f"File not found: {file_path}")
    
    print(f"\nFixed {fixed_count} files")

if __name__ == "__main__":
    main()