#!/usr/bin/env python3
"""Fix corrupted 'override {' patterns - override should come before the brace"""

import re
import os
import sys

def fix_override_brace(file_path):
    """Fix 'override {' to be just '{'"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # Pattern: override followed by opening brace
    # The override keyword should not appear right before an opening brace
    content = re.sub(r'\s*override\s*\{', ' {', content)
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Files identified by the extraction script
    files = [
        'ode_robots/motors/angularmotor.h',
        'ode_robots/motors/oneaxisservo.h',
        'ode_robots/osg/cameramanipulator.cpp',
        'ode_robots/sensors/irsensor.h',
        'ode_robots/sensors/relativepositionsensor.h',
        'ode_robots/simulations/vision/BallPlaying/main.cpp',
    ]
    
    fixed_count = 0
    for file_path in files:
        if os.path.exists(file_path):
            if fix_override_brace(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
            else:
                print(f"No changes needed: {file_path}")
        else:
            print(f"File not found: {file_path}")
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()