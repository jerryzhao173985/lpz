#!/usr/bin/env python3
"""Add explicit keyword to single-parameter constructors."""

import re
import os
import subprocess

def add_explicit_to_constructor(filepath, class_name, line_num):
    """Add explicit keyword to a constructor at specified line."""
    if not os.path.exists(filepath):
        return False
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    if line_num > 0 and line_num <= len(lines):
        line = lines[line_num - 1]
        # Check if it's a constructor and doesn't already have explicit
        if class_name in line and 'explicit' not in line:
            # Add explicit before the constructor
            lines[line_num - 1] = re.sub(rf'(\s*)({class_name}\s*\()', r'\1explicit \2', line)
            with open(filepath, 'w') as f:
                f.writelines(lines)
            return True
    
    return False

def main():
    # Get files with noExplicitConstructor warnings
    result = subprocess.run(
        ['cppcheck', '--enable=all', '--std=c++17', '--suppress=missingIncludeSystem', '--quiet', 'selforg'],
        stderr=subprocess.PIPE,
        text=True
    )
    
    constructors_to_fix = []
    for line in result.stderr.splitlines():
        if 'noExplicitConstructor' in line and "Class '" in line:
            parts = line.split(':')
            if len(parts) >= 3:
                filepath = parts[0]
                line_num = int(parts[1])
                # Extract class name from message
                match = re.search(r"Class '(\w+)'", line)
                if match:
                    class_name = match.group(1)
                    constructors_to_fix.append((filepath, line_num, class_name))
    
    print(f"Found {len(constructors_to_fix)} constructors needing explicit keyword")
    
    modified_count = 0
    for filepath, line_num, class_name in constructors_to_fix[:30]:  # Process first 30
        print(f"Fixing {filepath}:{line_num} - {class_name}")
        if add_explicit_to_constructor(filepath, class_name, line_num):
            modified_count += 1
            print("  ✓ Added explicit")
        else:
            print("  ✗ Failed")
    
    print(f"\nTotal constructors fixed: {modified_count}")

if __name__ == '__main__':
    main()