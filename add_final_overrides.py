#!/usr/bin/env python3
"""Add final override keywords to remaining virtual functions."""

import re
import os

def add_override_to_file(filepath):
    """Add override keyword to virtual functions missing it."""
    if not os.path.exists(filepath):
        return False
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    changes = []
    modified = content
    
    # Pattern to find virtual functions without override
    # Look for virtual functions that don't already have override
    patterns = [
        # Pattern for step functions
        (r'(virtual\s+void\s+step\s*\([^)]+\))\s*;', r'\1 override;'),
        (r'(virtual\s+void\s+stepNoLearning\s*\([^)]+\))\s*;', r'\1 override;'),
        # Pattern for other common virtual functions
        (r'(virtual\s+void\s+init\s*\([^)]+\))\s*;', r'\1 override;'),
        (r'(virtual\s+const\s+matrix::Matrix\s+learn\s*\([^)]+\))\s*;', r'\1 override;'),
    ]
    
    for pattern, replacement in patterns:
        matches = list(re.finditer(pattern, modified))
        for match in reversed(matches):
            # Check if it already has override
            if 'override' not in match.group(0):
                new_text = re.sub(pattern, replacement, match.group(0))
                modified = modified[:match.start()] + new_text + modified[match.end():]
                changes.append(('added override', match.start()))
    
    if changes:
        print(f"  Added {len(changes)} override keywords")
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(modified)
        return True
    
    return False

def main():
    # Target files with remaining override warnings
    target_files = [
        '/Users/jerry/lpzrobot_mac/selforg/controller/semox.h',
        '/Users/jerry/lpzrobot_mac/selforg/controller/switchcontroller.h',
        '/Users/jerry/lpzrobot_mac/selforg/controller/universalcontroller.h',
    ]
    
    modified_count = 0
    for filepath in target_files:
        print(f"\nProcessing: {filepath}")
        if add_override_to_file(filepath):
            modified_count += 1
    
    print(f"\nTotal files modified: {modified_count}")

if __name__ == '__main__':
    main()