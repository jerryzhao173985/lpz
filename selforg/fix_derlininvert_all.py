#!/usr/bin/env python3
"""Fix all override and explicit issues in derlininvert.cpp"""

import re

def fix_all_issues():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/derlininvert.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Fix const issues (e.g., "HY & const HY& HY")
    content = re.sub(r'(\w+)\s*&\s*const\s+\1&\s*\1', r'\1 & \1', content)
    
    # Fix specific error patterns
    content = re.sub(r'calcErrorFactor\(xsi,\s*\(const\s+logaE&\s*2\)\s*!=\s*0,\s*\(const\s+rootE&\s*2\)\s*!=\s*0\)', 
                     r'calcErrorFactor(xsi, false, false)', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed {file_path}")

fix_all_issues()