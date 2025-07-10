#!/usr/bin/env python3
import os
import re
import glob

# Additional files with override issues
specific_fixes = {
    '/Users/jerry/simulator/lpz/selforg/controller/neuralgas.h': [
        'printWeights',
        'printCellsizes'
    ]
}

def fix_override_issues(filepath, methods=None):
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except:
        return False
        
    original = content
    
    if methods:
        # Fix specific methods
        for method in methods:
            pattern = rf'(virtual\s+[\w\s\*&:<>]+\s+{method}\s*\([^)]*\)(?:\s*const)?\s*)override(\s*;)'
            content = re.sub(pattern, r'\1\2', content)
    else:
        # Generic fix for any virtual method with override that shouldn't have it
        # This will catch methods that don't override base class methods
        content = re.sub(
            r'(virtual\s+[\w\s\*&:<>]+\s+\w+\s*\([^)]*\)(?:\s*const)?\s*)override(\s*;)',
            r'\1\2',
            content
        )
    
    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

# Fix specific known issues
for filepath, methods in specific_fixes.items():
    if os.path.exists(filepath):
        if fix_override_issues(filepath, methods):
            print(f"Fixed override issues in {os.path.basename(filepath)}")

# Also add override where it's missing (like the learn method in neuralgas)
def add_missing_overrides(filepath):
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except:
        return False
        
    original = content
    
    # Add override to learn method if it's missing
    content = re.sub(
        r'(virtual\s+void\s+learn\s*\([^)]*\))\s*;',
        r'\1 override;',
        content
    )
    
    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

# Fix missing override in neuralgas.h
neuralgas_path = '/Users/jerry/simulator/lpz/selforg/controller/neuralgas.h'
if os.path.exists(neuralgas_path):
    if add_missing_overrides(neuralgas_path):
        print(f"Added missing override in {os.path.basename(neuralgas_path)}")