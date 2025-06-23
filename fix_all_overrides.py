#!/usr/bin/env python3
"""Fix ALL remaining override warnings comprehensively."""

import re
import os
import sys

def extract_override_warnings(log_file):
    """Extract all override warnings from build log."""
    warnings = {}
    with open(log_file, 'r') as f:
        for line in f:
            if 'override' in line and 'warning:' in line:
                match = re.match(r'([^:]+):(\d+):(\d+): warning: \'(\w+)\' overrides', line)
                if match:
                    filepath = match.group(1)
                    line_num = int(match.group(2))
                    method = match.group(4)
                    
                    if filepath not in warnings:
                        warnings[filepath] = []
                    warnings[filepath].append((line_num, method))
    
    return warnings

def fix_override_in_file(filepath, warnings_list):
    """Fix override warnings in a specific file."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    # Create backup
    backup_path = filepath + '.bak.override_fix'
    if not os.path.exists(backup_path):
        os.system(f'cp "{filepath}" "{backup_path}"')
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()
    
    modified = False
    
    # Sort warnings by line number in reverse order to avoid position shifts
    warnings_list.sort(key=lambda x: x[0], reverse=True)
    
    for line_num, method in warnings_list:
        if line_num <= len(lines):
            line_idx = line_num - 1
            line = lines[line_idx]
            
            # Skip if already has override
            if 'override' in line:
                continue
            
            # Pattern to find where to insert override
            # Handle various function declaration patterns
            patterns = [
                # Pattern 1: function ending with ;
                (r'(\b' + method + r'\s*\([^)]*\)(?:\s*const)?)\s*;', r'\1 override;'),
                # Pattern 2: function ending with {
                (r'(\b' + method + r'\s*\([^)]*\)(?:\s*const)?)\s*{', r'\1 override {'),
                # Pattern 3: pure virtual function
                (r'(\b' + method + r'\s*\([^)]*\)(?:\s*const)?)\s*=\s*0\s*;', r'\1 override = 0;'),
                # Pattern 4: multiline function declaration
                (r'(\b' + method + r'\s*\([^)]*\)(?:\s*const)?)\s*$', r'\1 override')
            ]
            
            for pattern, replacement in patterns:
                new_line = re.sub(pattern, replacement, line)
                if new_line != line:
                    lines[line_idx] = new_line
                    modified = True
                    print(f"  Fixed {method} at line {line_num}")
                    break
    
    if modified:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.writelines(lines)
        return True
    
    return False

def main():
    # Parse warnings from log file
    log_file = 'all_warnings.log'
    if not os.path.exists(log_file):
        print(f"Warning log file {log_file} not found!")
        return
    
    warnings = extract_override_warnings(log_file)
    
    print(f"Found {len(warnings)} files with override warnings")
    
    modified_count = 0
    for filepath, warnings_list in warnings.items():
        print(f"\nProcessing {filepath} ({len(warnings_list)} warnings):")
        
        # Adjust path if needed
        full_path = filepath
        if not os.path.exists(full_path) and not filepath.startswith('/'):
            full_path = os.path.join('/Users/jerry/lpzrobot_mac/selforg', filepath)
        
        if fix_override_in_file(full_path, warnings_list):
            modified_count += 1
    
    print(f"\nModified {modified_count} files")
    print("\nNow rebuild to verify all override warnings are fixed.")

if __name__ == '__main__':
    main()