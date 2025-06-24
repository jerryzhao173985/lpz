#!/usr/bin/env python3
"""Fix final compilation errors"""

import re
import os

def fix_derbigcontroller_h(file_path):
    """Fix method signatures in derbigcontroller.h to match implementations"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove const from matrix parameters that are modified
    content = re.sub(r'void kwtaInhibition\(const (matrix::Matrix&)', 
                     r'void kwtaInhibition(\1', content)
    content = re.sub(r'void limitC\(const (matrix::Matrix&)', 
                     r'void limitC(\1', content)
    
    with open(file_path, 'w') as f:
        f.write(content)

def fix_derlinunivers_h(file_path):
    """Fix DerLinUniversConf structure"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Find the struct and add missing members
    struct_pattern = r'(struct DerLinUniversConf\s*{[^}]*)(};)'
    
    def add_members(match):
        struct_body = match.group(1)
        
        # Add missing members if not present
        if 'init' not in struct_body:
            struct_body += '\n  double init = 1.0;'
        if 'squashsize' not in struct_body:
            struct_body += '\n  double squashsize = 0.05;'
        if 'someInternalParams' not in struct_body:
            struct_body += '\n  bool someInternalParams = true;'
        if 'motorlayer' not in struct_body:
            struct_body += '\n  int motorlayer = -1;'
        
        return struct_body + match.group(2)
    
    content = re.sub(struct_pattern, add_members, content, flags=re.DOTALL)
    
    with open(file_path, 'w') as f:
        f.write(content)

def fix_derlinunivers_cpp(file_path):
    """Fix remaining issues in derlinunivers.cpp"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    with open(file_path, 'w') as f:
        f.write(content)

def fix_derbigcontroller_cpp(file_path):
    """Fix remaining issues in derbigcontroller.cpp"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix for loops with override  
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix method signatures to match header
    content = re.sub(r'void\s+DerBigController::kwtaInhibition\(matrix::Matrix&', 
                     r'void DerBigController::kwtaInhibition(matrix::Matrix&', content)
    content = re.sub(r'void\s+DerBigController::limitC\(matrix::Matrix&', 
                     r'void DerBigController::limitC(matrix::Matrix&', content)
    
    with open(file_path, 'w') as f:
        f.write(content)

# Fix files
if os.path.exists("/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.h"):
    fix_derbigcontroller_h("/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.h")
    print("Fixed derbigcontroller.h")

if os.path.exists("/Users/jerry/lpzrobot_mac/selforg/controller/derlinunivers.h"):
    fix_derlinunivers_h("/Users/jerry/lpzrobot_mac/selforg/controller/derlinunivers.h")
    print("Fixed derlinunivers.h")

if os.path.exists("/Users/jerry/lpzrobot_mac/selforg/controller/derlinunivers.cpp"):
    fix_derlinunivers_cpp("/Users/jerry/lpzrobot_mac/selforg/controller/derlinunivers.cpp")
    print("Fixed derlinunivers.cpp")

if os.path.exists("/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.cpp"):
    fix_derbigcontroller_cpp("/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.cpp")
    print("Fixed derbigcontroller.cpp")