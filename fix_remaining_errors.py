#!/usr/bin/env python3
"""
Fix remaining compilation errors.
"""

import os
import re
import sys

def fix_remaining_errors_in_file(filepath):
    """Fix remaining compilation errors."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return 0
    
    original_content = content
    changes = 0
    
    # Fix 1: Remove override from const getName()
    pattern1 = re.compile(
        r'const\s+std::string&\s+getName\s*\(\s*\)\s*const\s+override',
        re.MULTILINE
    )
    if pattern1.search(content):
        content = pattern1.sub(r'const std::string& getName() const', content)
        changes += 1
    
    # Fix 2: Remove override from Inspectable methods that don't override
    # These are pure virtual in Inspectable base class
    inspectable_methods = ['getInternalParamNames', 'getInternalParams']
    for method in inspectable_methods:
        pattern = re.compile(
            rf'virtual\s+\w+\s+{method}\s*\([^)]*\)\s*const\s+override',
            re.MULTILINE
        )
        if pattern.search(content):
            content = pattern.sub(lambda m: m.group(0).replace(' override', ''), content)
            changes += 1
    
    # Fix 3: Remove override from NoiseGenerator::init
    pattern3 = re.compile(
        r'virtual\s+void\s+init\s*\(\s*unsigned\s+int\s+dimension[^)]*\)\s+override',
        re.MULTILINE
    )
    if pattern3.search(content):
        content = pattern3.sub(lambda m: m.group(0).replace(' override', ''), content)
        changes += 1
    
    # Fix 4: Add override to methods that need it (based on warnings)
    # Add override to NoiseGenerator methods
    noise_methods = [
        (r'(virtual\s+double\s+generate\s*\(\s*\))\s*{', r'\1 override {'),
        (r'(virtual\s+void\s+add\s*\([^)]*\))\s*{', r'\1 override {')
    ]
    
    for pattern_str, replacement in noise_methods:
        pattern = re.compile(pattern_str, re.MULTILINE)
        matches = pattern.findall(content)
        if matches:
            content = pattern.sub(replacement, content)
            changes += len(matches)
    
    # Fix 5: Add override to AbstractIAFController methods
    iaf_methods = [
        (r'(virtual\s+void\s+init\s*\(\s*int\s+sensornumber[^)]*\))\s*;', r'\1 override;'),
        (r'(virtual\s+void\s+step\s*\(\s*const\s+sensor\*[^)]*\))\s*;', r'\1 override;'),
        (r'(virtual\s+void\s+stepNoLearning\s*\(\s*const\s+sensor\*[^)]*\))\s*;', r'\1 override;')
    ]
    
    for pattern_str, replacement in iaf_methods:
        pattern = re.compile(pattern_str, re.MULTILINE)
        matches = pattern.findall(content)
        if matches and 'abstractiafcontroller' in filepath.lower():
            content = pattern.sub(replacement, content)
            changes += len(matches)
    
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
        'selforg/utils/plotoption.h',
        'selforg/utils/inspectable.h',
        'selforg/utils/noisegenerator.h',
        'selforg/controller/abstractiafcontroller.h',
        'selforg/include/selforg/plotoption.h',
        'selforg/include/selforg/inspectable.h',
        'selforg/include/selforg/noisegenerator.h',
        'selforg/include/selforg/abstractiafcontroller.h'
    ]
    
    total_changes = 0
    
    # Fix priority files
    for filepath in priority_files:
        if os.path.exists(filepath):
            total_changes += fix_remaining_errors_in_file(filepath)
    
    # Then process all other files
    dirs_to_process = ['selforg']
    
    for dir_name in dirs_to_process:
        if os.path.exists(dir_name):
            for root, dirs, files in os.walk(dir_name):
                if '.svn' in root:
                    continue
                for file in files:
                    if file.endswith(('.h', '.cpp')):
                        filepath = os.path.join(root, file)
                        if filepath not in priority_files:
                            changes = fix_remaining_errors_in_file(filepath)
                            total_changes += changes
    
    print(f"\nTotal errors fixed: {total_changes}")

if __name__ == '__main__':
    main()