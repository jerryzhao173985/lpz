#!/usr/bin/env python3
"""Comprehensive fix for noisegenerator.h"""

import re

def fix_noisegenerator():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/utils/noisegenerator.h"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix the malformed for loops
    content = re.sub(r'for \(for \(([^)]+)\) \{\)', r'for (\1)', content)
    
    # Fix "getDimension() const override" in base class
    content = re.sub(r'getDimension\(\) const\s+override', 'getDimension() const', content)
    
    # Fix override usage in ColorUniformNoise (it was missing sqrttau and mean1channel members)
    # Add missing member declarations after finding the class
    color_uniform_fix = """protected:
  double tau = 0;     // smoothing paramter
  double sqrttau = 0; // square root of tau
  double mean1channel = 0; // mean for single channel
  double* mean;"""
    
    content = re.sub(r'protected:\s*\n\s*double tau = 0;\s*// smoothing paramter\s*\n\s*double\* mean;',
                     color_uniform_fix, content)
    
    # Now add override to derived class methods where appropriate
    # For generate() methods in derived classes
    derived_classes = [
        ('NoNoise', 'generate'),
        ('WhiteUniformNoise', 'generate'),
        ('WhiteNormalNoise', 'generate'),
        ('ColorUniformNoise', 'generate'),
        ('ColorNormalNoise', 'generate'),
        ('SineWhiteNoise', 'generate')
    ]
    
    for class_name, method in derived_classes:
        # Pattern to find the method in the specific class
        pattern = rf'(class {class_name}[^{{]*{{[^}}]*virtual double {method}\(\))\s*{{'
        replacement = r'\1 override {'
        content = re.sub(pattern, replacement, content, flags=re.DOTALL)
    
    # Fix add() override in derived classes that override it
    add_override_classes = ['ColorUniformNoise', 'ColorNormalNoise', 'SineWhiteNoise']
    for class_name in add_override_classes:
        pattern = rf'(class {class_name}[^{{]*{{[^}}]*virtual void add\([^)]+\))\s*{{'
        replacement = r'\1 override {'
        content = re.sub(pattern, replacement, content, flags=re.DOTALL)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_noisegenerator()