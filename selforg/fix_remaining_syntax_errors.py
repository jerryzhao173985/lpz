#!/usr/bin/env python3
"""Fix remaining syntax errors"""

import re
import os

def fix_derpseudosensor_h(file_path):
    """Fix static const and override issues in derpseudosensor.h"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix static method with const
    content = re.sub(r'(static\s+\w+(?:\s*::\s*\w+)*\s+\w+\s*\([^)]*\))\s*const\s*{', r'\1 {', content)
    
    # Remove override from methods that don't override
    content = re.sub(r'(void\s+setHeadPosition\s*\([^)]*\))\s*override', r'\1', content)
    content = re.sub(r'(void\s+setTrunkPosition\s*\([^)]*\))\s*override', r'\1', content)
    
    with open(file_path, 'w') as f:
        f.write(content)

def fix_derbigcontroller_cpp(file_path):
    """Fix limitC signature issue"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix DerBigController::limitC signature
    content = re.sub(r'void DerBigController::limitC\(const matrix::Matrix&', 
                     r'void DerBigController::limitC(matrix::Matrix&', content)
    
    with open(file_path, 'w') as f:
        f.write(content)

def fix_derpseudosensor_cpp(file_path):
    """Fix Position to double conversion"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix Position to double conversions
    content = re.sub(r'l \+= static_cast<double>\(headPosition\)\.z;', 
                     r'l += headPosition.z;', content)
    content = re.sub(r'l \+= static_cast<double>\(trunkPosition\)\.z;', 
                     r'l += trunkPosition.z;', content)
    
    with open(file_path, 'w') as f:
        f.write(content)

def fix_elman_cpp(file_path):
    """Fix syntax error in elman.cpp"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Add missing semicolon
    content = re.sub(r'(MultiLayerFFNN::init\([^)]+\))(\s+override)', r'\1;', content)
    content = re.sub(r'(MultiLayerFFNN::init\([^)]+\))([^;])', r'\1;\2', content)
    
    with open(file_path, 'w') as f:
        f.write(content)

def fix_esn_h(file_path):
    """Fix static const and syntax issues in esn.h"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix static method with const
    content = re.sub(r'(static\s+ESNConf\s+getDefaultConf\s*\([^)]*\))\s*const\s*{', r'\1 {', content)
    
    # Fix syntax errors in method declarations
    content = re.sub(r'(getInternalParamNames\s*\(\s*\))\s*c\s*override', r'\1 const override', content)
    
    with open(file_path, 'w') as f:
        f.write(content)

# Fix files
files_to_fix = [
    ("/Users/jerry/lpzrobot_mac/selforg/controller/derpseudosensor.h", fix_derpseudosensor_h),
    ("/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.cpp", fix_derbigcontroller_cpp),
    ("/Users/jerry/lpzrobot_mac/selforg/controller/derpseudosensor.cpp", fix_derpseudosensor_cpp),
    ("/Users/jerry/lpzrobot_mac/selforg/controller/elman.cpp", fix_elman_cpp),
    ("/Users/jerry/lpzrobot_mac/selforg/controller/esn.h", fix_esn_h)
]

for file_path, fix_func in files_to_fix:
    if os.path.exists(file_path):
        fix_func(file_path)
        print(f"Fixed {file_path}")
    else:
        print(f"File not found: {file_path}")