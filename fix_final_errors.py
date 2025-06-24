#!/usr/bin/env python3
"""
Fix final compilation errors.
"""

import os
import re
import sys

def fix_final_errors_in_file(filepath):
    """Fix final compilation errors."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return 0
    
    original_content = content
    changes = 0
    
    # Fix 1: Remove override from NoiseGenerator::add that shouldn't have it
    if 'noisegenerator' in filepath.lower():
        # Remove override from first add method (line ~81)
        pattern1 = re.compile(
            r'(virtual\s+void\s+add\s*\(\s*double\*\s*value,\s*double\s+noiseStrength\s*\))\s*override\s*{',
            re.MULTILINE
        )
        if pattern1.search(content):
            content = pattern1.sub(r'\1 {', content)
            changes += 1
        
        # Add override to init methods that need it
        pattern2 = re.compile(
            r'(virtual\s+void\s+init\s*\(\s*unsigned\s+int\s+dimension[^)]*\))(?!\s*override)\s*{',
            re.MULTILINE
        )
        matches = pattern2.findall(content)
        if matches:
            content = pattern2.sub(r'\1 override {', content)
            changes += len(matches)
    
    # Fix 2: Remove override from AbstractWiring::reset
    if 'abstractwiring' in filepath.lower():
        pattern3 = re.compile(
            r'virtual\s+void\s+reset\s*\(\s*\)\s*override',
            re.MULTILINE
        )
        if pattern3.search(content):
            content = pattern3.sub('virtual void reset()', content)
            changes += 1
    
    # Fix 3: Add override to MotorBabbler methods
    if 'motorbabbler' in filepath.lower():
        patterns = [
            (r'(virtual\s+void\s+init\s*\([^)]*\))(?!\s*override)\s*;', r'\1 override;'),
            (r'(virtual\s+void\s+stepNoLearning\s*\([^)]*\))(?!\s*override)\s*;', r'\1 override;')
        ]
        for pattern_str, replacement in patterns:
            pattern = re.compile(pattern_str, re.MULTILINE)
            matches = pattern.findall(content)
            if matches:
                content = pattern.sub(replacement, content)
                changes += len(matches)
    
    # Fix 4: Fix nullptr comparisons in wiredcontroller.cpp
    if filepath.endswith('wiredcontroller.cpp'):
        # Fix NULL comparisons with proper nullptr usage
        pattern4 = re.compile(
            r'(callBack(?:motor|sensor)\s*!=\s*)NULL\b',
            re.MULTILINE
        )
        if pattern4.search(content):
            content = pattern4.sub(r'\1nullptr', content)
            changes += 1
    
    # Fix 5: Add override to AbstractIAFController::notifyOnChange
    if 'abstractiafcontroller' in filepath.lower():
        pattern5 = re.compile(
            r'(virtual\s+void\s+notifyOnChange\s*\(\s*const\s+paramkey&\s*key\s*\))(?!\s*override)\s*;',
            re.MULTILINE
        )
        if pattern5.search(content):
            content = pattern5.sub(r'\1 override;', content)
            changes += 1
    
    if content != original_content:
        try:
            with open(filepath, 'w') as f:
                f.write(content)
            print(f"Fixed {changes} errors in {filepath}")
        except Exception as e:
            print(f"Error writing {filepath}: {e}")
            return 0
    
    return changes

def main():
    # Priority files to fix
    priority_files = [
        'selforg/utils/noisegenerator.h',
        'selforg/wirings/abstractwiring.h',
        'selforg/controller/motorbabbler.h',
        'selforg/wiredcontroller.cpp',
        'selforg/controller/abstractiafcontroller.h',
        'selforg/include/selforg/noisegenerator.h',
        'selforg/include/selforg/abstractwiring.h',
        'selforg/include/selforg/motorbabbler.h',
        'selforg/include/selforg/abstractiafcontroller.h'
    ]
    
    total_changes = 0
    
    # Fix priority files
    for filepath in priority_files:
        if os.path.exists(filepath):
            total_changes += fix_final_errors_in_file(filepath)
    
    print(f"\nTotal errors fixed: {total_changes}")

if __name__ == '__main__':
    main()