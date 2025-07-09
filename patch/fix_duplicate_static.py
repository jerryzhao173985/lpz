#!/usr/bin/env python3
"""Fix duplicate static keywords introduced by the previous script"""

import re
import os

def fix_duplicate_static(file_path):
    """Remove duplicate static keywords"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # Fix patterns with multiple static keywords
    patterns = [
        (r'static\s+static\s+static\s+static\s+', 'static '),
        (r'static\s+static\s+static\s+', 'static '),
        (r'static\s+static\s+', 'static '),
        (r'static\s+double\s+explicit\s+', 'static double '),
        (r'double\s+explicit\s+', 'double '),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content)
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Files that were modified
    files_to_fix = [
        'selforg/controller/sox.h',
        'selforg/controller/sos.h',
        'selforg/controller/homeokinbase.h',
        'ode_robots/simulations/hexapod/sox.h',
        'selforg/controller/learning_strategy.h',
        'selforg/controller/regularisation.h',
        'selforg/controller/apexsox.h',
        'selforg/controller/itsox.h',
        'selforg/controller/soxexpand.h',
        'selforg/controller/pimax.h',
        'selforg/controller/invertnchannelcontroller.h',
        'ode_robots/simulations/life_vs_copy/pimax.h',
        'ode_robots/simulations/vision/BallPlaying2/soxignorenull.h',
    ]
    
    fixed_count = 0
    
    print("Fixing duplicate static keywords...")
    for file_path in files_to_fix:
        if os.path.exists(file_path):
            if fix_duplicate_static(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()