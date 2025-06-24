#!/usr/bin/env python3
"""
Fix C++ warnings in ode_robots component
"""
import os
import re
import subprocess
import sys

def run_cppcheck_on_file(filepath):
    """Run cppcheck on a single file and return warnings"""
    cmd = [
        'cppcheck',
        '--enable=warning,style,performance',
        '--std=c++17',
        '--suppress=missingIncludeSystem',
        '--suppress=unmatchedSuppression',
        '--suppress=syntaxError',  # Too many false positives
        '--quiet',
        '--template={file}:{line}:{severity}:{id}:{message}',
        filepath
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=10)
        return result.stderr.strip().split('\n') if result.stderr else []
    except subprocess.TimeoutExpired:
        return []
    except Exception as e:
        print(f"Error running cppcheck on {filepath}: {e}")
        return []

def fix_shadow_variables(filepath):
    """Fix shadowed variable warnings"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    modified = False
    
    # Common shadow patterns
    patterns = [
        # Constructor parameter shadows member
        (r'(\w+)\s*\(\s*(\w+)\s+(\w+)\s*\)\s*:\s*(\w+)\s*\(\s*\3\s*\)',
         r'\1(\2 \3_) : \4(\3_)'),
        
        # Function parameter shadows member variable
        (r'void\s+set(\w+)\s*\(\s*(\w+)\s+(\w+)\s*\)\s*{\s*this->(\w+)\s*=\s*\3;',
         r'void set\1(\2 \3_) { this->\4 = \3_;'),
    ]
    
    for pattern, replacement in patterns:
        if re.search(pattern, content):
            content = re.sub(pattern, replacement, content)
            modified = True
    
    if modified:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def fix_unused_parameters(filepath):
    """Fix unused parameter warnings"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Add [[maybe_unused]] to parameters in virtual functions
    pattern = r'virtual\s+\w+\s+\w+\s*\(([^)]+)\)'
    
    def add_maybe_unused(match):
        params = match.group(1)
        # Don't modify if already has attributes
        if '[[' in params:
            return match.group(0)
        
        # Add [[maybe_unused]] to each parameter
        new_params = []
        for param in params.split(','):
            param = param.strip()
            if param and not param.startswith('[['):
                # Extract type and name
                parts = param.rsplit(' ', 1)
                if len(parts) == 2:
                    new_params.append(f"[[maybe_unused]] {param}")
                else:
                    new_params.append(param)
            else:
                new_params.append(param)
        
        return f"virtual {match.group(0)[7:].split('(')[0]}({', '.join(new_params)})"
    
    modified = False
    if 'virtual' in content and '[[maybe_unused]]' not in content:
        new_content = re.sub(pattern, add_maybe_unused, content)
        if new_content != content:
            content = new_content
            modified = True
    
    if modified:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def fix_float_comparisons(filepath):
    """Fix float comparison warnings"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Add epsilon comparison for float/double
    patterns = [
        # if (value == 0.0)
        (r'if\s*\(\s*(\w+)\s*==\s*0\.0\s*\)', r'if (fabs(\1) < 1e-9)'),
        (r'if\s*\(\s*(\w+)\s*==\s*0\s*\)', r'if (fabs(\1) < 1e-9)'),
        # if (value != 0.0)
        (r'if\s*\(\s*(\w+)\s*!=\s*0\.0\s*\)', r'if (fabs(\1) > 1e-9)'),
        (r'if\s*\(\s*(\w+)\s*!=\s*0\s*\)', r'if (fabs(\1) > 1e-9)'),
    ]
    
    modified = False
    
    # Check if it's likely dealing with floats
    if any(keyword in content for keyword in ['float', 'double', 'dReal']):
        for pattern, replacement in patterns:
            if re.search(pattern, content):
                content = re.sub(pattern, replacement, content)
                modified = True
        
        # Add math.h include if needed
        if modified and 'fabs' in content and '#include <math.h>' not in content:
            # Add after other includes
            include_pos = content.find('#include')
            if include_pos != -1:
                end_of_line = content.find('\n', include_pos)
                content = content[:end_of_line+1] + '#include <math.h>\n' + content[end_of_line+1:]
    
    if modified:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def process_file(filepath):
    """Process a single file for warnings"""
    if not os.path.exists(filepath):
        return False
    
    # Skip very large files
    if os.path.getsize(filepath) > 500000:  # 500KB
        return False
    
    print(f"Checking {filepath}")
    
    # Get warnings for this file
    warnings = run_cppcheck_on_file(filepath)
    
    fixed = False
    
    for warning in warnings:
        if not warning:
            continue
            
        if 'shadowVariable' in warning:
            if fix_shadow_variables(filepath):
                print(f"  Fixed shadow variables")
                fixed = True
                
        elif 'unusedParameter' in warning:
            if fix_unused_parameters(filepath):
                print(f"  Fixed unused parameters")
                fixed = True
                
        elif 'compareValueWithBool' in warning or 'knownCondition' in warning:
            if fix_float_comparisons(filepath):
                print(f"  Fixed float comparisons")
                fixed = True
    
    return fixed

def main():
    os.chdir('/Users/jerry/lpzrobot_mac/ode_robots')
    
    # Find all C++ files
    cpp_files = []
    for root, dirs, files in os.walk('.'):
        # Skip certain directories
        if any(skip in root for skip in ['.svn', 'build', 'CMakeFiles', 'obsolete']):
            continue
            
        for file in files:
            if file.endswith(('.cpp', '.h')):
                cpp_files.append(os.path.join(root, file))
    
    print(f"Found {len(cpp_files)} C++ files to check")
    
    # Process files in batches
    fixed_count = 0
    for i, filepath in enumerate(cpp_files[:50]):  # Process first 50 files
        if i % 10 == 0:
            print(f"\nProgress: {i}/{len(cpp_files)}")
        
        if process_file(filepath):
            fixed_count += 1
    
    print(f"\nFixed warnings in {fixed_count} files")

if __name__ == '__main__':
    main()