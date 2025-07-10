#!/usr/bin/env python3
import os
import re

# Additional files with override issues from latest CI
override_fixes = {
    '/Users/jerry/simulator/lpz/selforg/controller/som.h': ['printWeights'],
    '/Users/jerry/simulator/lpz/selforg/controller/soml.h': ['learnModelBP']
}

def fix_override_issues(filepath, methods):
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except:
        return False
        
    original = content
    
    for method in methods:
        # Remove override from these methods
        pattern = rf'(virtual\s+[\w\s\*&:<>]+\s+{method}\s*\([^)]*\)(?:\s*const)?\s*)override(\s*;)'
        content = re.sub(pattern, r'\1\2', content)
    
    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

# Fix all known issues
for filepath, methods in override_fixes.items():
    if os.path.exists(filepath):
        if fix_override_issues(filepath, methods):
            print(f"Fixed override issues in {os.path.basename(filepath)}")
        else:
            print(f"No changes needed for {os.path.basename(filepath)}")

print("\nSearching for any other potential override issues...")

# Let's also scan for more potential issues
import glob
controller_headers = glob.glob('/Users/jerry/simulator/lpz/selforg/controller/*.h')

# Methods that commonly have override issues
problematic_methods = [
    'printWeights',
    'printCellsizes', 
    'learnModelBP',
    'calcCandHUpdates',
    'setAandCandCalcH_xsi',
    'calculateMIs',
    'calculateH_x',
    'calculateH_yx',
    'updateXsiFreqMatrixList',
    'calculateH_Xsi',
    'updateMIs',
    'getState',
    'fillSensorBuffer',
    'fillMotorBuffer'
]

for header in controller_headers:
    if header not in override_fixes:
        if fix_override_issues(header, problematic_methods):
            print(f"Fixed additional override issues in {os.path.basename(header)}")