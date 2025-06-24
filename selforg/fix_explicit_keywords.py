#!/usr/bin/env python3
"""Fix all remaining explicit keyword issues"""

import os
import re

def fix_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Fix "explicit for" -> "for"
    content = re.sub(r'explicit\s+for\s*\(', 'for (', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix "explicit switch" -> "switch"
    content = re.sub(r'explicit\s+switch\s*\(', 'switch (', content)
    
    # Fix "explicit while" -> "while" (just in case)
    content = re.sub(r'explicit\s+while\s*\(', 'while (', content)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed {file_path}")
        return True
    return False

def main():
    base_dir = "/Users/jerry/lpzrobot_mac/selforg"
    files_with_issues = [
        "utils/controller_misc.cpp",
        "utils/plotoptionengine.cpp",
        "utils/discretisizer.cpp",
        "statistictools/statistictools.cpp",
        "statistictools/measure/trackablemeasure.cpp",
        "statistictools/measure/statisticmeasure.cpp",
        "controller/semox.cpp",
        "controller/switchcontroller.cpp",
        "controller/splitcontrol.cpp",
        "controller/measureadapter.cpp"
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