#!/usr/bin/env python3
"""
Script to fix C-style casts with proper C++ casts
"""

import re
import os
import sys
from pathlib import Path

class CastFixer:
    def __init__(self, verbose=False):
        self.files_processed = 0
        self.casts_fixed = 0
        self.verbose = verbose
        
    def detect_cast_type(self, cast_expr, target_type):
        """Determine the appropriate C++ cast type"""
        # Common patterns
        if 'const' in target_type and 'const' not in cast_expr:
            return 'const_cast'
        elif target_type == 'void*' or cast_expr.endswith('*'):
            if 'malloc' in cast_expr or 'calloc' in cast_expr:
                return 'static_cast'
            else:
                return 'reinterpret_cast'
        elif any(t in target_type for t in ['int', 'float', 'double', 'char', 'bool', 'size_t']):
            return 'static_cast'
        else:
            return 'static_cast'  # Default safe choice
    
    def fix_cstyle_casts(self, content):
        """Replace C-style casts with appropriate C++ casts"""
        modified = False
        
        # Pattern for C-style casts
        # Matches (type)expression or (type*)expression
        cast_pattern = re.compile(
            r'\((\s*(?:const\s+)?[A-Za-z_][\w:]*(?:\s*\*)*)\s*\)\s*([^;,\)\s]+)',
            re.MULTILINE
        )
        
        def replace_cast(match):
            nonlocal modified
            type_part = match.group(1).strip()
            expr_part = match.group(2).strip()
            
            # Skip function-like macros
            if expr_part.startswith('(') and expr_part.endswith(')'):
                return match.group(0)
            
            # Skip if it's already a C++ cast
            if any(c in match.group(0) for c in ['static_cast', 'const_cast', 'reinterpret_cast', 'dynamic_cast']):
                return match.group(0)
            
            # Determine cast type
            cast_type = self.detect_cast_type(expr_part, type_part)
            
            # Format the replacement
            replacement = f'{cast_type}<{type_part}>({expr_part})'
            
            if self.verbose:
                print(f"  Replacing: {match.group(0)} -> {replacement}")
            
            modified = True
            return replacement
        
        # Apply replacements
        new_content = cast_pattern.sub(replace_cast, content)
        
        if new_content != content:
            self.casts_fixed += cast_pattern.findall(content).__len__()
            return new_content, True
        
        return content, False
    
    def process_file(self, filepath):
        """Process a single file to fix C-style casts"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return False
        
        original_content = content
        
        # Count C-style casts before
        before_count = len(re.findall(r'\([A-Za-z_][\w:]*\s*\*?\s*\)', content))
        
        # Fix casts
        content, modified = self.fix_cstyle_casts(content)
        
        if modified:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                after_count = len(re.findall(r'\([A-Za-z_][\w:]*\s*\*?\s*\)', content))
                fixed = before_count - after_count
                print(f"Fixed {filepath}: {fixed} casts")
                return True
            except Exception as e:
                print(f"Error writing {filepath}: {e}")
                return False
        
        return False
    
    def process_directory(self, directory):
        """Process all C++ files in directory"""
        for root, dirs, files in os.walk(directory):
            # Skip build directories
            dirs[:] = [d for d in dirs if d not in ['build', 'build_opt', 'build_dbg', '.git']]
            
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp')):
                    filepath = os.path.join(root, file)
                    self.files_processed += 1
                    self.process_file(filepath)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_all_cstyle_casts.py <directory> [--verbose]")
        sys.exit(1)
    
    verbose = '--verbose' in sys.argv
    fixer = CastFixer(verbose=verbose)
    
    path = sys.argv[1]
    if os.path.isfile(path):
        fixer.files_processed = 1
        fixer.process_file(path)
    elif os.path.isdir(path):
        fixer.process_directory(path)
    else:
        print(f"Error: {path} is not a valid file or directory")
        sys.exit(1)
    
    print(f"\nProcessed {fixer.files_processed} files")
    print(f"Fixed {fixer.casts_fixed} C-style casts")

if __name__ == "__main__":
    main()