#!/usr/bin/env python3
"""Fix override and syntax issues in homeokinbase.h"""

import re

def fix_homeokinbase():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/homeokinbase.h"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix the split parameter name "number_steps_for_av override eraging_"
    content = re.sub(r'int number_steps_for_av override eraging_', 'int number_steps_for_averaging_', content)
    
    # Remove override from methods that don't override
    content = re.sub(r'virtual matrix::Matrix calculateDelayedValues\(([^)]+)\)\s*override', 
                     r'virtual matrix::Matrix calculateDelayedValues(\1)', content)
    
    content = re.sub(r'virtual double calcErrorFactor\(([^)]+)\)\s*override', 
                     r'virtual double calcErrorFactor(\1)', content)
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_homeokinbase()