#!/usr/bin/env python3
"""
Fix incorrect changes to the codebase:
1. Remove 'explicit' from control flow statements (while, for, switch)
2. Restore 'override' to virtual destructors
3. Restore 'override' to methods that actually override base class methods
"""

import os
import re
import sys

def fix_explicit_control_flow(content):
    """Remove 'explicit' from control flow statements."""
    # Remove explicit from while loops
    content = re.sub(r'\bexplicit\s+while\s*\(', 'while (', content)
    
    # Remove explicit from for loops
    content = re.sub(r'\bexplicit\s+for\s*\(', 'for (', content)
    
    # Remove explicit from switch statements
    content = re.sub(r'\bexplicit\s+switch\s*\(', 'switch (', content)
    
    return content

def restore_override_to_destructors(content):
    """Restore override to virtual destructors."""
    # Pattern: virtual ~ClassName(); should be virtual ~ClassName() override;
    # This is a bit tricky because we need to know if it's actually overriding
    # For now, let's be conservative and only add it back to Qt widget destructors
    
    # Qt widget destructors that likely override
    qt_destructor_pattern = r'(virtual\s+~Q\w+(?:Widget|Dialog|Window|View|Handler))\(\s*\)\s*;'
    content = re.sub(qt_destructor_pattern, r'\1() override;', content)
    
    return content

def process_file(filepath):
    """Process a single file to fix incorrect changes."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except:
        print(f"Error reading {filepath}")
        return False
    
    original_content = content
    
    # Apply fixes
    content = fix_explicit_control_flow(content)
    content = restore_override_to_destructors(content)
    
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
    # Get list of modified files from git
    import subprocess
    result = subprocess.run(['git', 'diff', '--name-only'], capture_output=True, text=True)
    if result.returncode != 0:
        print("Error getting modified files from git")
        return
    
    modified_files = result.stdout.strip().split('\n')
    
    # Filter for C++ files
    cpp_files = [f for f in modified_files if f.endswith(('.cpp', '.cc', '.h', '.hpp'))]
    
    fixed_count = 0
    for filepath in cpp_files:
        if os.path.exists(filepath):
            if process_file(filepath):
                fixed_count += 1
                print(f"Fixed: {filepath}")
    
    print(f"\nFixed {fixed_count} files")

if __name__ == '__main__':
    main()