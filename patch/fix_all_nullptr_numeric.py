#!/usr/bin/env python3
"""Fix all nullptr used in numeric contexts where 0 should be used"""

import re
import os
import subprocess

def find_files_with_nullptr_loops():
    """Find all files with for loops using nullptr"""
    try:
        result = subprocess.run(
            ['grep', '-r', '-l', '--include=*.cpp', '--include=*.h', '--include=*.hpp', 
             r'for\s*([^)]*=\s*nullptr\s*;', '.'],
            capture_output=True,
            text=True
        )
        files = [f.strip() for f in result.stdout.split('\n') if f.strip()]
        return files
    except:
        return []

def fix_nullptr_in_file(file_path):
    """Fix nullptr incorrectly used in numeric contexts"""
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
    except:
        print(f"Error reading {file_path}")
        return False
    
    original = content
    
    # Fix various for loop patterns with nullptr
    patterns = [
        # for(int i = nullptr; ...) -> for(int i = 0; ...)
        (r'for\s*\(\s*int\s+(\w+)\s*=\s*nullptr\s*;', r'for (int \1 = 0;'),
        # for(size_t i = nullptr; ...) -> for(size_t i = 0; ...)
        (r'for\s*\(\s*size_t\s+(\w+)\s*=\s*nullptr\s*;', r'for (size_t \1 = 0;'),
        # for(unsigned i = nullptr; ...) -> for(unsigned i = 0; ...)
        (r'for\s*\(\s*unsigned\s+(\w+)\s*=\s*nullptr\s*;', r'for (unsigned \1 = 0;'),
        # for(dReal i = nullptr; ...) -> for(dReal i = 0; ...)
        (r'for\s*\(\s*dReal\s+(\w+)\s*=\s*nullptr\s*;', r'for (dReal \1 = 0;'),
        # Generic numeric type
        (r'for\s*\(\s*(int|size_t|unsigned|long|short|double|float|dReal)\s+(\w+)\s*=\s*nullptr\s*;', 
         r'for (\1 \2 = 0;'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content)
    
    if content != original:
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        except:
            print(f"Error writing {file_path}")
            return False
    return False

def main():
    print("Finding files with nullptr in numeric contexts...")
    
    # Get files from grep
    files = find_files_with_nullptr_loops()
    
    if not files:
        # Fallback to manual search
        files = []
        for root, dirs, filenames in os.walk('.'):
            # Skip certain directories
            skip_dirs = ['.git', '.cache', 'build', 'dist', 'backup_']
            dirs[:] = [d for d in dirs if not any(skip in d for skip in skip_dirs)]
            
            for filename in filenames:
                if filename.endswith(('.cpp', '.h', '.hpp')):
                    files.append(os.path.join(root, filename))
    
    print(f"Found {len(files)} potential files to check")
    
    fixed_count = 0
    checked_count = 0
    
    for file_path in files:
        if os.path.exists(file_path):
            checked_count += 1
            if fix_nullptr_in_file(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
            
            # Progress indicator
            if checked_count % 100 == 0:
                print(f"Checked {checked_count} files...")
    
    print(f"\nTotal files fixed: {fixed_count} out of {checked_count} checked")

if __name__ == "__main__":
    main()