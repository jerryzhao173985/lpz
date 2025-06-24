#!/usr/bin/env python3
"""Fix all override issues in classicreinforce.cpp"""

import re

def fix_all_overrides():
    file_path = "/Users/jerry/lpzrobot_mac/selforg/controller/classicreinforce.cpp"
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Remove all instances of override used as statement terminator
    content = re.sub(r'\s+override;', ';', content)
    
    # Fix for loops with override
    content = re.sub(r'for\s*\(([^)]+)\)\s*override\s*{', r'for (\1) {', content)
    
    # Fix "explicit if" -> "if"
    content = re.sub(r'explicit\s+if\s*\(', 'if (', content)
    
    # Add missing member variables to ClassicReinforceConf
    # Let me also fix references to conf.numContext and conf.reinforce_interval
    # which don't exist in the struct
    content = re.sub(r'conf\.numContext', '0 /* conf.numContext */', content)
    content = re.sub(r'&conf\.reinforce_interval', '&reinforce_interval', content)
    content = re.sub(r'conf\.reinforce_interval', 'reinforce_interval', content)
    
    with open(file_path, 'w') as f:
        f.write(content)
    
    print(f"Fixed all overrides in {file_path}")

fix_all_overrides()