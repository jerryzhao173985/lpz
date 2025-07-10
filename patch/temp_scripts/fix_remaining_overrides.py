#!/usr/bin/env python3
"""
Fix remaining incorrect override keywords in controller headers.
"""

import os
import re

# Dictionary of files and methods that need override removed
fixes = {
    'selforg/controller/mutualinformationcontroller.h': [
        'setAandCandCalcH_xsi',
        'calculateMIs',
        'calculateH_x', 
        'calculateH_yx',
        'updateXsiFreqMatrixList',
        'calculateH_Xsi',
        'updateMIs',
        'getState'
    ],
    'selforg/controller/multireinforce.h': [
        'fillSensorBuffer',
        'fillMotorBuffer'
    ]
}

def remove_override(file_path, method_names):
    """Remove override keyword from specific methods in a file."""
    if not os.path.exists(file_path):
        print(f"Warning: {file_path} does not exist")
        return
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    for method in method_names:
        # Match virtual method declarations with override
        # This pattern handles multi-line declarations
        pattern = rf'(virtual\s+.*\s+{method}\s*\([^)]*\)[^;{{]*)(\s+override)(\s*[;{{])'
        replacement = r'\1\3'
        content = re.sub(pattern, replacement, content, flags=re.MULTILINE | re.DOTALL)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed {file_path}")
    else:
        print(f"No changes needed for {file_path}")

def main():
    """Main function to fix all files."""
    print("Fixing remaining incorrect override keywords...")
    
    for file_path, methods in fixes.items():
        remove_override(file_path, methods)
    
    print("\nDone fixing override keywords.")

if __name__ == '__main__':
    main()