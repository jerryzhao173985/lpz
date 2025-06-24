#!/usr/bin/env python3
"""Fix if static_cast syntax errors"""

import os
import re
import glob

def fix_file(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except:
        print(f"Error reading {file_path}")
        return False
    
    original_content = content
    
    # Fix "if static_cast<var>" patterns
    # Pattern 1: if static_cast<var>(expression)
    content = re.sub(r'if\s+static_cast<([^>]+)>\s*\(([^)]+)\)', r'if (\1)', content)
    
    # Pattern 2: if static_cast<randGen>(this)->randGen = randGen
    content = re.sub(r'if\s+static_cast<randGen>\(this\)->randGen\s*=\s*randGen', 
                     r'if (randGen) this->randGen = randGen', content)
    
    # Pattern 3: simple if static_cast<var> without parens
    content = re.sub(r'if\s+static_cast<([^>]+)>\s*([^(])', r'if (\1) \2', content)
    
    # Fix if (const var& value) patterns
    content = re.sub(r'if\s*\(\s*const\s+([^&]+)&\s+([^)]+)\)', r'if (\1 & \2)', content)
    
    if content != original_content:
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Fixed {file_path}")
            return True
        except:
            print(f"Error writing {file_path}")
            return False
    return False

def main():
    base_dir = "/Users/jerry/lpzrobot_mac/selforg"
    
    # Find all .cpp and .h files
    cpp_files = glob.glob(os.path.join(base_dir, "**/*.cpp"), recursive=True)
    h_files = glob.glob(os.path.join(base_dir, "**/*.h"), recursive=True)
    
    all_files = cpp_files + h_files
    
    # Skip Python files and backup files
    all_files = [f for f in all_files if not f.endswith('.py') and not f.endswith('~')]
    
    print(f"Found {len(all_files)} C++ files to check")
    
    fixed_count = 0
    for file_path in all_files:
        if fix_file(file_path):
            fixed_count += 1
    
    print(f"\nFixed {fixed_count} files")

if __name__ == "__main__":
    main()