#!/usr/bin/env python3
"""
Fix compilation errors from incorrect override placement.
"""

import os
import re
import sys

def fix_override_errors_in_file(filepath):
    """Fix compilation errors related to override keyword."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return 0
    
    original_content = content
    changes = 0
    
    # Fix 1: Remove override from non-virtual functions
    # Pattern: const method with override
    pattern1 = re.compile(
        r'(\s+)const\s+(\w+[&*]?)\s+(\w+)\s*\([^)]*\)\s*const\s+override\s*;',
        re.MULTILINE
    )
    
    def fix_const_override(match):
        nonlocal changes
        indent = match.group(1)
        return_type = match.group(2)
        func_name = match.group(3)
        # Check if this is likely a getter (getName, etc)
        if func_name.startswith('get'):
            changes += 1
            return match.group(0).replace(' override', '')
        return match.group(0)
    
    content = pattern1.sub(fix_const_override, content)
    
    # Fix 2: Remove override from non-virtual print function
    pattern2 = re.compile(
        r'void\s+print\s*\([^)]*\)\s*const\s+override\s*;',
        re.MULTILINE
    )
    content = pattern2.sub(lambda m: m.group(0).replace(' override', ''), content)
    if pattern2.search(original_content):
        changes += 1
    
    # Fix 3: Fix malformed override statements
    # Pattern: statement) override; should be statement);
    pattern3 = re.compile(
        r'(\w+\([^)]*\))\s+override\s*;',
        re.MULTILINE
    )
    
    def fix_malformed_override(match):
        nonlocal changes
        statement = match.group(1)
        # Check if this looks like a function call, not a declaration
        if not re.search(r'^\s*(virtual\s+)?[\w:]+[\s&*]+\w+\s*\(', statement):
            changes += 1
            return statement + ';'
        return match.group(0)
    
    content = pattern3.sub(fix_malformed_override, content)
    
    # Fix 4: Fix duplicate override in stepNoLearning
    pattern4 = re.compile(
        r'stepNoLearning\([^)]*\)\s*override\s*{\s*stepNoLearning\([^)]*\)\s*override\s*;',
        re.MULTILINE | re.DOTALL
    )
    content = pattern4.sub(lambda m: m.group(0).replace('override;', ';'), content)
    if pattern4.search(original_content):
        changes += 1
    
    # Fix 5: Remove override from methods that don't override base class
    # Methods in Configurable that don't override Parametrizable
    configurable_non_override = [
        'notifyOnChange', 'getParam', 'setParam', 'getParamList',
        'addConfigurable', 'removeConfigurable'
    ]
    
    for method in configurable_non_override:
        pattern = re.compile(
            rf'virtual\s+\w+[\s&*]*{method}\s*\([^)]*\)(?:\s*const)?\s+override',
            re.MULTILINE
        )
        if pattern.search(content):
            content = pattern.sub(lambda m: m.group(0).replace(' override', ''), content)
            changes += 1
    
    if content != original_content:
        try:
            with open(filepath, 'w') as f:
                f.write(content)
            print(f"Fixed {changes} override errors in {filepath}")
        except Exception as e:
            print(f"Error writing {filepath}: {e}")
            return 0
    
    return changes

def main():
    # Priority files to fix based on the compilation errors
    priority_files = [
        'selforg/utils/plotoption.h',
        'selforg/utils/configurable.h',
        'selforg/include/selforg/plotoption.h',
        'selforg/include/selforg/configurable.h',
        'selforg/include/selforg/motorbabbler.h',
        'selforg/controller/soxexpand.cpp',
        'selforg/wiredcontroller.h',
        'selforg/agent.h'
    ]
    
    total_changes = 0
    
    # Fix priority files first
    for filepath in priority_files:
        if os.path.exists(filepath):
            total_changes += fix_override_errors_in_file(filepath)
    
    # Then process all other files
    dirs_to_process = ['selforg', 'ode_robots']
    
    for dir_name in dirs_to_process:
        if os.path.exists(dir_name):
            for root, dirs, files in os.walk(dir_name):
                if '.svn' in root:
                    continue
                for file in files:
                    if file.endswith(('.h', '.cpp')):
                        filepath = os.path.join(root, file)
                        if filepath not in priority_files:
                            changes = fix_override_errors_in_file(filepath)
                            total_changes += changes
    
    print(f"\nTotal override errors fixed: {total_changes}")

if __name__ == '__main__':
    main()