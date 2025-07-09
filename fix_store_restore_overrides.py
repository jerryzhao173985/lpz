#\!/usr/bin/env python3
"""
Fix incorrect override keywords on store/restore methods.
These methods exist in Storeable interface but controllers that inherit 
from multiple interfaces (like ESN from FeedForwardNN and Storeable) 
shouldn't have override on store/restore.
"""

import os
import re

# Files that have store/restore with override that shouldn't
files_to_fix = [
    'selforg/controller/esn.h',
    'selforg/controller/remotecontrolled.h',
    'selforg/controller/neuralgas.h',
    'selforg/controller/derlininvert.h',
    'selforg/controller/derinf.h',
    'selforg/controller/som.h',
    'selforg/controller/multilayerffnn.h',
    'selforg/controller/onelayerffnn.h',
    'selforg/controller/braitenberg.h',
    'selforg/controller/controllernet.h',
    'selforg/controller/derpseudosensor.h',
    'selforg/controller/replaycontroller.h',
    'selforg/controller/derlinunivers.h',
    'selforg/controller/elman.h',
    'selforg/controller/modelwithmemoryadapter.h',
    'selforg/controller/dep.h',
]

def remove_override_from_store_restore(file_path):
    """Remove override keyword from store/restore methods."""
    if not os.path.exists(file_path):
        print(f"Warning: {file_path} does not exist")
        return
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Pattern to match store/restore methods with override
    patterns = [
        (r'(virtual\s+bool\s+store\s*\([^)]*\)\s*const)\s+override(\s*[;{])', r'\1\2'),
        (r'(virtual\s+bool\s+restore\s*\([^)]*\))\s+override(\s*[;{])', r'\1\2'),
    ]
    
    for pattern, replacement in patterns:
        content = re.sub(pattern, replacement, content, flags=re.MULTILINE | re.DOTALL)
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"Fixed {file_path}")
    else:
        print(f"No changes needed for {file_path}")

def main():
    """Main function to fix all files."""
    print("Fixing incorrect override keywords on store/restore methods...")
    
    for file_path in files_to_fix:
        remove_override_from_store_restore(file_path)
    
    print("\nDone fixing store/restore override keywords.")

if __name__ == '__main__':
    main()
