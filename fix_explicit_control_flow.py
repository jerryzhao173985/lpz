#!/usr/bin/env python3
"""
Fix incorrect 'explicit' keywords added to control flow statements.
"""

import os
import re
import subprocess

def fix_explicit_in_file(filepath):
    """Remove 'explicit' from control flow statements in a file."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except:
        print(f"Error reading {filepath}")
        return False
    
    original_content = content
    
    # Remove explicit from while loops
    content = re.sub(r'\bexplicit\s+while\s*\(', 'while (', content)
    
    # Remove explicit from for loops  
    content = re.sub(r'\bexplicit\s+for\s*\(', 'for (', content)
    
    # Remove explicit from switch statements
    content = re.sub(r'\bexplicit\s+switch\s*\(', 'switch (', content)
    
    # Only write if changed
    if content != original_content:
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        except:
            print(f"Error writing {filepath}")
            return False
    
    return False

def main():
    # Find all files with "explicit while", "explicit for", or "explicit switch"
    patterns = ['explicit while', 'explicit for', 'explicit switch']
    
    all_files = set()
    
    for pattern in patterns:
        result = subprocess.run(['grep', '-r', '-l', pattern, '.'], 
                              capture_output=True, text=True)
        if result.returncode == 0:
            files = result.stdout.strip().split('\n')
            all_files.update(f for f in files if f and f.endswith(('.cpp', '.cc', '.h', '.hpp')))
    
    fixed_count = 0
    for filepath in sorted(all_files):
        if os.path.exists(filepath) and not filepath.startswith('./opende/'):
            if fix_explicit_in_file(filepath):
                fixed_count += 1
                print(f"Fixed: {filepath}")
    
    print(f"\nFixed {fixed_count} files")

if __name__ == '__main__':
    main()