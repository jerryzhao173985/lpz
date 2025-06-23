#!/usr/bin/env python3
"""Fix uninitialized members in MyRobot classes across simulation files."""

import re
import os
import subprocess

def find_myrobot_files():
    """Find files containing MyRobot class with uninitialized members."""
    result = subprocess.run(
        ['grep', '-l', 'class MyRobot', '-r', 'simulations/', '--include=*.cpp'],
        capture_output=True,
        text=True
    )
    return [f.strip() for f in result.stdout.splitlines() if f.strip()]

def get_uninit_members(filepath):
    """Run cppcheck to find uninitialized members."""
    result = subprocess.run(
        ['cppcheck', '--enable=warning', '--suppress=missingInclude', filepath],
        capture_output=True,
        text=True
    )
    
    uninit_members = []
    for line in result.stderr.splitlines():
        if 'MyRobot::' in line and 'uninitMemberVar' in line:
            # Extract member name
            match = re.search(r"MyRobot::(\w+)'", line)
            if match:
                uninit_members.append(match.group(1))
    
    return uninit_members

def fix_myrobot_constructor(filepath, uninit_members):
    """Fix MyRobot constructor to initialize all members."""
    if not uninit_members:
        return False
        
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Find MyRobot constructor
    # Pattern to match constructor initialization list
    pattern = r'(MyRobot\s*\([^)]*\)\s*:\s*AbstractRobot[^{]+)(\{)'
    
    def replace_constructor(match):
        init_list = match.group(1)
        opening_brace = match.group(2)
        
        # Check if members are already initialized
        for member in uninit_members:
            if member not in init_list:
                # Add initialization
                init_list = init_list.rstrip() + f", {member}(0)"
                
        return init_list + " " + opening_brace
    
    new_content = re.sub(pattern, replace_constructor, content)
    
    if new_content != content:
        # Backup
        with open(filepath + '.bak_myrobot', 'w') as f:
            f.write(content)
        
        # Write fixed content
        with open(filepath, 'w') as f:
            f.write(new_content)
        
        print(f"✓ Fixed {filepath}: initialized {', '.join(uninit_members)}")
        return True
    
    return False

def main():
    """Main function to fix all MyRobot uninitialized members."""
    print("=== Fixing MyRobot Uninitialized Members ===\n")
    
    files = find_myrobot_files()
    print(f"Found {len(files)} files with MyRobot class\n")
    
    total_fixed = 0
    for filepath in files:
        uninit_members = get_uninit_members(filepath)
        if uninit_members:
            print(f"Checking {filepath}: {len(uninit_members)} uninitialized members")
            if fix_myrobot_constructor(filepath, uninit_members):
                total_fixed += 1
    
    print(f"\n=== Summary ===")
    print(f"Files fixed: {total_fixed}")
    print(f"Backups created with .bak_myrobot extension")

if __name__ == "__main__":
    main()