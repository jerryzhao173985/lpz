#!/usr/bin/env python3
"""
Fix missing override specifiers in C++ code.
This script adds 'override' keyword to virtual functions that override base class methods.
"""

import os
import re
import sys

def fix_override_in_file(filepath):
    """Fix missing override specifiers in a single file."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return 0
    
    original_content = content
    changes = 0
    
    # Pattern to match virtual function declarations that should have override
    # This matches functions that:
    # 1. Have 'virtual' keyword
    # 2. Don't already have 'override'
    # 3. End with ; or { (declaration or definition)
    # 4. Are not pure virtual (= 0)
    
    # Common virtual functions that typically override base class methods
    override_functions = [
        'init', 'step', 'stepNoLearning', 'sense', 'getMotorsSensors',
        'getName', 'getRevision', 'clone', 'store', 'restore',
        'notifyOnChange', 'command', 'create', 'destroy', 'update',
        'place', 'setMotors', 'getSensors', 'getSensorsIntern',
        'getMotorsIntern', 'setMotorsIntern', 'setSensorsIntern',
        'doInternalStuff', 'cleanup', 'setColor', 'getColor',
        'getPrimitive', 'getMainPrimitive', 'setTexture',
        'addParameter', 'removeParameter', 'getParameter',
        'getAllParameters', 'setParam', 'getParam', 'getParamList',
        'print', 'getStructure', 'getLayerNames', 'getConnections',
        'damp', 'learn', 'getInputDim', 'getOutputDim',
        'process', 'processSensors', 'processMotors',
        'wireMotors', 'wireSensors', 'reset', 'getInternalParams',
        'getInternalParamNames', 'getInternalParam',
        'setInternalParam', 'getInternalParamDimension',
        'addInspectableValue', 'addInspectableMatrix',
        'removeInspectable', 'setNameOfInspectable',
        'addConfigurable', 'removeConfigurable',
        'videoFrames', 'printInternalParameters'
    ]
    
    # Build pattern for known override functions
    func_pattern = '|'.join(override_functions)
    
    # Pattern 1: Virtual function declarations without override
    pattern1 = re.compile(
        r'(\s*virtual\s+.*?\s+(' + func_pattern + r')\s*\([^)]*\)(?:\s*const)?)\s*;(?!\s*//.*override)',
        re.MULTILINE | re.DOTALL
    )
    
    # Pattern 2: Virtual function definitions without override  
    pattern2 = re.compile(
        r'(\s*virtual\s+.*?\s+(' + func_pattern + r')\s*\([^)]*\)(?:\s*const)?)\s*\{',
        re.MULTILINE | re.DOTALL
    )
    
    # Replace declarations
    def replace_declaration(match):
        nonlocal changes
        decl = match.group(1)
        # Check if it's not a pure virtual function
        if '= 0' not in match.group(0):
            changes += 1
            return decl + ' override;'
        return match.group(0)
    
    # Replace definitions
    def replace_definition(match):
        nonlocal changes
        decl = match.group(1)
        changes += 1
        return decl + ' override {'
    
    content = pattern1.sub(replace_declaration, content)
    content = pattern2.sub(replace_definition, content)
    
    # Pattern 3: Fix specific cases where virtual is in the .h file but implementation is in .cpp
    # Look for function implementations that match our known override functions
    if filepath.endswith('.cpp'):
        # Pattern for non-virtual function implementations that should have override
        impl_pattern = re.compile(
            r'^(\s*(?!virtual|static).*?\s+\w+::(' + func_pattern + r')\s*\([^)]*\)(?:\s*const)?)\s*\{',
            re.MULTILINE
        )
        
        def check_impl(match):
            # Don't add override to .cpp implementations
            # The override should be in the header file
            return match.group(0)
        
        content = impl_pattern.sub(check_impl, content)
    
    if content != original_content:
        try:
            with open(filepath, 'w') as f:
                f.write(content)
            print(f"Fixed {changes} override warnings in {filepath}")
        except Exception as e:
            print(f"Error writing {filepath}: {e}")
            return 0
    
    return changes

def main():
    if len(sys.argv) > 1:
        # Process specific files
        total_changes = 0
        for filepath in sys.argv[1:]:
            if os.path.isfile(filepath) and (filepath.endswith('.h') or filepath.endswith('.cpp')):
                total_changes += fix_override_in_file(filepath)
        print(f"\nTotal override specifiers added: {total_changes}")
    else:
        # Process all .h and .cpp files in selforg and ode_robots
        total_changes = 0
        dirs_to_process = ['selforg', 'ode_robots']
        
        for dir_name in dirs_to_process:
            if os.path.exists(dir_name):
                for root, dirs, files in os.walk(dir_name):
                    # Skip .svn directories
                    if '.svn' in root:
                        continue
                    for file in files:
                        if file.endswith(('.h', '.cpp')):
                            filepath = os.path.join(root, file)
                            total_changes += fix_override_in_file(filepath)
        
        print(f"\nTotal override specifiers added: {total_changes}")

if __name__ == '__main__':
    main()