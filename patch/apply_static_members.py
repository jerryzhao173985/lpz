#!/usr/bin/env python3
"""Add static keyword to member functions that don't access instance state"""

import re
import os

def should_be_static(function_content):
    """Check if a function should be static based on its content"""
    
    # Functions that typically should be static
    static_patterns = [
        r'\bg\s*\(\s*double\s+z\s*\)',  # g(double z) transfer function
        r'\bg_s\s*\(\s*double\s+z\s*\)',  # g_s(double z) derivative
        r'\bclip\s*\(',  # clip functions
        r'\bsigmoid\s*\(',  # sigmoid functions
        r'\btanh\s*\(',  # tanh functions
        r'\bpower\s*\(',  # power functions
    ]
    
    # Check if it matches common static function patterns
    for pattern in static_patterns:
        if re.search(pattern, function_content):
            return True
    
    # Check if function doesn't access 'this' or member variables
    if 'this->' not in function_content and not re.search(r'\b(m_|_)\w+', function_content):
        # Simple functions that just do calculations
        if 'return' in function_content and ('+' in function_content or '*' in function_content or 'tanh' in function_content):
            return True
    
    return False

def add_static_to_functions(file_path):
    """Add static keyword to appropriate member functions"""
    
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # Common patterns for functions that should be static
    patterns = [
        # Transfer function g(double z)
        (r'(\s+)(double\s+g\s*\(\s*double\s+z\s*\)\s*{[^}]+})', r'\1static \2'),
        # Derivative g_s(double z)
        (r'(\s+)(double\s+g_s\s*\(\s*double\s+z\s*\)\s*{[^}]+})', r'\1static \2'),
        # Clip functions
        (r'(\s+)(double\s+clip\s*\([^)]+\)\s*{[^}]+})', r'\1static \2'),
        # Helper functions that don't use instance data
        (r'(\s+)((?:double|float|int|bool)\s+(?:sigmoid|tanh_|power|clip\w*)\s*\([^)]+\)\s*{[^}]+})', r'\1static \2'),
    ]
    
    for pattern, replacement in patterns:
        # Only add static if not already present
        if re.search(pattern, content) and 'static' not in re.search(pattern, content).group(0):
            content = re.sub(pattern, replacement, content)
    
    # Special handling for multi-line static functions
    # Match function declarations like:
    #   double g(double z) {
    #     return tanh(z);
    #   }
    multiline_pattern = r'(\n\s*)(double\s+g\s*\(\s*double\s+z\s*\)\s*{\s*\n\s*return\s+tanh\s*\(\s*z\s*\)\s*;\s*\n\s*})'
    matches = re.finditer(multiline_pattern, content)
    for match in matches:
        if 'static' not in match.group(0):
            content = content.replace(match.group(0), match.group(1) + 'static ' + match.group(2))
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    return False

def main():
    # Priority files with static member functions
    target_files = [
        'selforg/controller/sox.h',
        'selforg/controller/sos.h',
        'selforg/controller/derpseudosensor.h',
        'selforg/controller/homeokinbase.h',
        'selforg/controller/invertmotornstep.h',
        'selforg/controller/invertmotorspace.h',
        'ode_robots/simulations/barrel/homeostatic/invertnchannelfw.h',
        'ode_robots/simulations/hexapod/sox.h',
        'ode_robots/simulations/humanoid_DEP/randomdyn.h',
    ]
    
    fixed_count = 0
    
    print("Adding static to member functions...")
    
    # Fix known files first
    for file_path in target_files:
        if os.path.exists(file_path):
            if add_static_to_functions(file_path):
                print(f"Fixed: {file_path}")
                fixed_count += 1
    
    # Search for more controller files
    controller_dirs = [
        'selforg/controller/',
        'ode_robots/simulations/',
    ]
    
    for dir_path in controller_dirs:
        if os.path.exists(dir_path):
            for root, dirs, files in os.walk(dir_path):
                for file in files:
                    if file.endswith(('.h', '.hpp')):
                        file_path = os.path.join(root, file)
                        if file_path not in target_files:
                            # Check if file contains g(double z) pattern
                            try:
                                with open(file_path, 'r', encoding='utf-8') as f:
                                    content = f.read()
                                    if 'g(double z)' in content or 'g_s(double z)' in content:
                                        if add_static_to_functions(file_path):
                                            print(f"Fixed: {file_path}")
                                            fixed_count += 1
                            except:
                                pass
    
    print(f"\nTotal files fixed: {fixed_count}")

if __name__ == "__main__":
    main()