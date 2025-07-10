#!/usr/bin/env python3
import os
import re

# Files and methods with override issues from CI
override_issues = {
    '/Users/jerry/simulator/lpz/selforg/controller/mutualinformationcontroller.h': [
        'setAandCandCalcH_xsi',
        'calculateMIs',
        'calculateH_x',
        'calculateH_yx',
        'updateXsiFreqMatrixList',
        'calculateH_Xsi',
        'updateMIs',
        'getState'
    ],
    '/Users/jerry/simulator/lpz/selforg/controller/multireinforce.h': [
        'fillSensorBuffer',
        'fillMotorBuffer'
    ]
}

def fix_override_issues(filepath, methods):
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except:
        return False
        
    original = content
    
    for method in methods:
        # Match virtual method declaration with override
        # This regex handles various return types and parameter lists
        pattern = rf'(virtual\s+[\w\s\*&:<>]+\s+{method}\s*\([^)]*\)(?:\s*const)?\s*)override(\s*;)'
        content = re.sub(pattern, r'\1\2', content)
    
    if content != original:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

# Fix all known issues
for filepath, methods in override_issues.items():
    if os.path.exists(filepath):
        if fix_override_issues(filepath, methods):
            print(f"Fixed override issues in {os.path.basename(filepath)}")

# Also search for any other similar patterns in the selforg/controller directory
import glob
controller_files = glob.glob('/Users/jerry/simulator/lpz/selforg/controller/*.h')

# List of methods that commonly have override issues
problematic_methods = [
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
    'fillMotorBuffer',
    'doDiscretisizeSensorValues',
    'doDiscretisizeMotorValues',
    'findMinAndMaxSensorRange',
    'findMinAndMaxMotorRange',
    'findMinAndMaxSensorValues',
    'findMinAndMaxMotorValues',
    'roundValue'
]

for file in controller_files:
    if file not in override_issues:
        if fix_override_issues(file, problematic_methods):
            print(f"Fixed additional override issues in {os.path.basename(file)}")