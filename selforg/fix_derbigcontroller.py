#!/usr/bin/env python3
"""Fix issues in derbigcontroller.h"""

import re

def fix_derbigcontroller():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/derbigcontroller.h"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix static method with const
    content = re.sub(r'static DerBigControllerConf getDefaultConf\(\)\s*const\s*{', 
                     r'static DerBigControllerConf getDefaultConf() {', content)
    
    # Fix "matrix::const Matrix&" -> "const matrix::Matrix&"
    content = re.sub(r'matrix::const\s+Matrix&', r'const matrix::Matrix&', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_derbigcontroller()