#!/usr/bin/env python3
import re
import sys

def fix_sprintf_in_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Find all sprintf occurrences
    sprintf_pattern = r'sprintf\s*\(\s*([^,]+),\s*(.+?)\);'
    
    def replace_sprintf(match):
        buffer_name = match.group(1).strip()
        format_and_args = match.group(2).strip()
        return f'snprintf({buffer_name}, sizeof({buffer_name}), {format_and_args});'
    
    # Replace all sprintf with snprintf
    new_content = re.sub(sprintf_pattern, replace_sprintf, content)
    
    # Special case for sprintf in keylist[written] 
    keylist_pattern = r'sprintf\s*\(\s*keylist\[written\],\s*(.+?)\);'
    def replace_keylist_sprintf(match):
        format_and_args = match.group(1).strip()
        return f'snprintf(keylist[written], keyLen, {format_and_args});'
    
    new_content = re.sub(keylist_pattern, replace_keylist_sprintf, new_content)
    
    if new_content != content:
        with open(filepath, 'w') as f:
            f.write(new_content)
        print(f"Fixed sprintf in {filepath}")
        return True
    return False

if __name__ == "__main__":
    filepath = "/Users/jerry/lpzrobot_mac/selforg/utils/controller_misc.cpp"
    if fix_sprintf_in_file(filepath):
        print("Successfully fixed sprintf warnings")
    else:
        print("No changes made")