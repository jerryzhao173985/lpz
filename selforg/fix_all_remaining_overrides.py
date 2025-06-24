#!/usr/bin/env python3
"""Fix all remaining override issues in the entire selforg directory"""

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
    
    # Remove all instances of override used as statement terminator
    content = re.sub(r'\s+override;', ';', content)
    
    # Fix "explicit for" -> "for"
    content = re.sub(r'explicit\s+for\s*\(', 'for (', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix "explicit switch" -> "switch"
    content = re.sub(r'explicit\s+switch\s*\(', 'switch (', content)
    
    # Fix "explicit while" -> "while"
    content = re.sub(r'explicit\s+while\s*\(', 'while (', content)
    
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
    
    # Fix logaE and rootE patterns
    content = re.sub(r'\(const logaE& (\d+)\) != 0', r'(logaE >= \1)', content)
    content = re.sub(r'\(const rootE& (\d+)\) != 0', r'(rootE >= \1)', content)
    
    # Fix matrix::const Matrix& -> const matrix::Matrix&
    content = re.sub(r'matrix::const\s+Matrix&', 'const matrix::Matrix&', content)
    
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