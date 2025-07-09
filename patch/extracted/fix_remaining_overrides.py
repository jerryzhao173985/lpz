#!/usr/bin/env python3
"""
Fix remaining misplaced override keywords after semicolons.
This targets the specific pattern where override appears after semicolons.
"""

import os
import re
import sys

def fix_override_after_semicolon(content):
    """Fix patterns where override appears after semicolon."""
    # Pattern 1: ) override;
    content = re.sub(r'\)\s+override\s*;', r') override;', content)
    
    # Pattern 2: const override;
    content = re.sub(r'const\s+override\s*;', r' const override;', content)
    
    # Pattern 3: Fix the specific misplaced pattern after declaration
    # virtual ~ECB() override; -> virtual ~ECB();
    content = re.sub(r'(virtual\s+~?\w+\([^)]*\)(?:\s+const)?)\s+override\s*;', r'\1;', content)
    
    # Pattern 4: Fix method declarations with override after semicolon
    content = re.sub(r'(virtual\s+\w+(?:<[^>]+>)?\s+\w+\([^)]*\)(?:\s+const)?)\s+override\s*;', r'\1 override;', content)
    
    return content

def process_file(filepath):
    """Process a single file to fix override issues."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original = content
        content = fix_override_after_semicolon(content)
        
        if content != original:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False

def main():
    # Target directories
    directories = ['ecbrobots/', 'ode_robots/', 'selforg/']
    
    fixed_files = []
    
    for directory in directories:
        if not os.path.exists(directory):
            continue
            
        for root, dirs, files in os.walk(directory):
            # Skip hidden directories
            dirs[:] = [d for d in dirs if not d.startswith('.')]
            
            for file in files:
                if file.endswith(('.h', '.hpp', '.cpp', '.cc')):
                    filepath = os.path.join(root, file)
                    if process_file(filepath):
                        fixed_files.append(filepath)
    
    print(f"Fixed {len(fixed_files)} files:")
    for f in sorted(fixed_files):
        print(f"  - {f}")
    
    return 0 if fixed_files else 1

if __name__ == "__main__":
    sys.exit(main())