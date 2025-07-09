#!/usr/bin/env python3
import os
import re

# Files that still have override issues according to the CI
files_with_override_issues = [
    '/Users/jerry/simulator/lpz/selforg/controller/discretecontrolleradapter.h',
]

def fix_override_issues(filepath):
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except:
        return False
        
    original = content
    
    # Remove override from these specific methods that don't override base class
    methods_to_fix = [
        'setIntervalCount',
        'setSensorIntervalCount', 
        'setMotorIntervalCount',
        'setIntervalRange',
        'doDiscretisizeSensorValues',
        'doDiscretisizeMotorValues',
        'findMinAndMaxSensorRange',
        'findMinAndMaxMotorRange',
        'findMinAndMaxSensorValues',
        'findMinAndMaxMotorValues',
        'roundValue'
    ]
    
    for method in methods_to_fix:
        # Match virtual method declaration with override
        pattern = rf'(virtual\s+\w+(?:\s*\*)?(?:\s+\w+::)?\s+{method}\s*\([^)]*\)(?:\s*const)?\s*)override(\s*;)'
        content = re.sub(pattern, r'\1\2', content)
    
    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

# Fix the specific file
for filepath in files_with_override_issues:
    if os.path.exists(filepath):
        if fix_override_issues(filepath):
            print(f"Fixed override issues in {os.path.basename(filepath)}")

# Also search for any other files that might have similar issues
import glob
for file in glob.glob('/Users/jerry/simulator/lpz/selforg/controller/*.h'):
    if file not in files_with_override_issues:
        if fix_override_issues(file):
            print(f"Fixed override issues in {os.path.basename(file)}")