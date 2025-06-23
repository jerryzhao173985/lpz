#!/usr/bin/env python3
"""
Script to fix remaining C-style casts more carefully
"""

import re
import os
import sys
from pathlib import Path

class SafeCastFixer:
    def __init__(self, verbose=False):
        self.files_processed = 0
        self.casts_fixed = 0
        self.verbose = verbose
        
    def fix_cstyle_casts(self, content):
        """Replace C-style casts with appropriate C++ casts"""
        modified = False
        lines = content.split('\n')
        new_lines = []
        
        for line_num, line in enumerate(lines):
            # Skip comments and preprocessor directives
            if line.strip().startswith('//') or line.strip().startswith('#') or line.strip().startswith('*'):
                new_lines.append(line)
                continue
                
            # Skip lines that are part of a comment block
            if '/*' in line or '*/' in line or line.strip().startswith('*'):
                new_lines.append(line)
                continue
            
            # Pattern for C-style casts - more precise
            # Matches (type) but not function calls like func(args)
            new_line = line
            
            # Simple cast pattern: (type)value
            pattern1 = re.compile(r'\(([A-Za-z_][\w:]*\s*\*?)\)\s*([a-zA-Z_][\w\.\->]*)')
            matches = list(pattern1.finditer(new_line))
            
            for match in reversed(matches):  # Process from right to left
                type_part = match.group(1).strip()
                expr_part = match.group(2).strip()
                
                # Skip if it looks like a function call
                if '(' in expr_part:
                    continue
                    
                # Skip if already a C++ cast
                if any(c in new_line[max(0, match.start()-15):match.start()] 
                       for c in ['static_cast', 'const_cast', 'reinterpret_cast', 'dynamic_cast']):
                    continue
                
                # Determine cast type
                if type_part.endswith('*'):
                    if 'malloc' in new_line or 'calloc' in new_line:
                        cast_type = 'static_cast'
                    else:
                        cast_type = 'reinterpret_cast'
                else:
                    cast_type = 'static_cast'
                
                # Replace
                replacement = f'{cast_type}<{type_part}>({expr_part})'
                new_line = new_line[:match.start()] + replacement + new_line[match.end():]
                modified = True
                self.casts_fixed += 1
                
                if self.verbose:
                    print(f"  Line {line_num + 1}: {match.group(0)} -> {replacement}")
            
            new_lines.append(new_line)
        
        return '\n'.join(new_lines), modified
    
    def process_file(self, filepath):
        """Process a single file to fix C-style casts"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return False
        
        original_content = content
        
        # Fix casts
        content, modified = self.fix_cstyle_casts(content)
        
        if modified:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"Fixed {filepath}: {self.casts_fixed} casts")
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
                    self.casts_fixed = 0  # Reset per file
                    self.process_file(filepath)

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 fix_remaining_casts.py <directory> [--verbose]")
        sys.exit(1)
    
    verbose = '--verbose' in sys.argv
    fixer = SafeCastFixer(verbose=verbose)
    
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

if __name__ == "__main__":
    main()