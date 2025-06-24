#!/usr/bin/env python3
"""Fix override issues in multilayerffnn.h"""

import re

def fix_multilayerffnn():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/multilayerffnn.h"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove override from methods that don't override anything in the base class
    # These methods are not in FeedForwardNN or InvertableModel
    methods_to_fix = [
        'getLayerOutput',
        'getLayerNum', 
        'getLayer',
        'getWeights',
        'getBias',
        'setSomeInternalParams'
    ]
    
    for method in methods_to_fix:
        # Match both const and non-const versions
        content = re.sub(rf'({method}\([^)]*\)(?:\s*const)?)\s*override', r'\1', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_multilayerffnn()