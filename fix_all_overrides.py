#!/usr/bin/env python3
"""
Fix incorrect override keywords in controller headers.
These methods don't exist in base classes but were incorrectly marked with override.
"""

import os
import re

# Dictionary of files and methods that need override removed
# Format: filename: [list of method names]
fixes = {
    'selforg/controller/crossmotorcoupling.h': ['setCMC', 'getCMC'],
    'selforg/controller/multilayerffnn.h': ['setActivationFunctions'],
    'selforg/controller/controllernet.h': ['response', 'responsePart', 'responseLinear', 'calcResponseIntern'],
    'selforg/controller/elman.h': ['weightIncrement', 'updateWeights'],
    'selforg/controller/derbigcontroller.h': ['learnController', 'management'],
    'selforg/controller/invertmotornstep.h': ['setSensorTeachingSignal', 'setReinforcement'],
    'selforg/controller/derlininvert.h': ['setSensorTeachingSignal', 'learnController', 'management'],
    'selforg/controller/derpseudosensor.h': ['setSensorTeachingSignal', 'learnController', 'management'],
    'selforg/controller/derinf.h': ['learnController', 'management'],
    'selforg/controller/pimax.h': ['setA', 'seth', 'calculateControllerValues'],
    'selforg/controller/dercontroller.h': ['calcErrorFactor'],
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
        pattern = rf'(virtual\s+.*\s+{method}\s*\([^)]*\)[^;{{]*)\s+override(\s*[;{{])'
        replacement = r'\1\2'
        content = re.sub(pattern, replacement, content, flags=re.MULTILINE | re.DOTALL)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed {file_path}")
    else:
        print(f"No changes needed for {file_path}")

def main():
    """Main function to fix all files."""
    print("Fixing incorrect override keywords in controller headers...")
    
    for file_path, methods in fixes.items():
        remove_override(file_path, methods)
    
    print("\nDone fixing override keywords.")

if __name__ == '__main__':
    main()