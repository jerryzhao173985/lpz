#!/usr/bin/env python3
"""Fix all remaining override issues in utils and other directories"""

import os
import re

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Remove all instances of override used as statement terminator
    content = re.sub(r'\s+override;', ';', content)
    
    # Fix "explicit for" -> "for"
    content = re.sub(r'explicit\s+for\s*\(', 'for (', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix "explicit switch" -> "switch"
    content = re.sub(r'explicit\s+switch\s*\(', 'switch (', content)
    
    # Fix function calls with override
    content = re.sub(r'(\)\s*)\s*override\s*;', r'\1;', content)
    
    # Fix assert statements with override
    content = re.sub(r'(assert\([^)]+\))\s*override\s*;', r'\1;', content)
    
    # Fix variable declarations/operations with override
    content = re.sub(r'(=\s*[^;]+)\s*override\s*;', r'\1;', content)
    
    # Fix for loops with override at the end
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix return statements with override
    content = re.sub(r'return\s+([^;]+)\s*override\s*;', r'return \1;', content)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed {file_path}")
        return True
    return False

def main():
    base_dir = "/Users/jerry/lpzrobot_mac/selforg"
    files_with_issues = [
        "tests/configurabletest.cpp",
        "utils/inspectable.cpp",
        "utils/backcaller.cpp",
        "utils/randomgenerator.cpp",
        "utils/configurablelist.cpp",
        "utils/plotoption.cpp",
        "utils/storeable.cpp",
        "utils/globaldatabase.cpp",
        "utils/trackrobots.cpp",
        "utils/configurable.cpp"
    ]
    
    fixed_count = 0
    for filename in files_with_issues:
        file_path = os.path.join(base_dir, filename)
        if os.path.exists(file_path):
            if fix_file(file_path):
                fixed_count += 1
        else:
            print(f"File not found: {file_path}")
    
    print(f"\nFixed {fixed_count} files")

if __name__ == "__main__":
    main()