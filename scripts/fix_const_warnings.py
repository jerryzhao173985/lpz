#!/usr/bin/env python3
"""
Script to fix const reference warnings systematically
"""

import re
import os
import sys
from pathlib import Path

class ConstReferenceFixer:
    def __init__(self):
        self.files_processed = 0
        self.fixes_applied = 0
        
    def fix_auto_reference(self, content):
        """Fix 'auto&' that should be 'const auto&' in range-based loops"""
        # Pattern to match for(auto& var : container) where var is not modified
        pattern = re.compile(r'for\s*\(\s*auto\s*&\s*(\w+)\s*:\s*([^)]+)\)')
        
        fixes = []
        for match in pattern.finditer(content):
            var_name = match.group(1)
            loop_start = match.start()
            
            # Find the loop body
            body_start = content.find('{', loop_start)
            if body_start == -1:
                continue
                
            # Find matching closing brace
            brace_count = 1
            body_end = body_start + 1
            while brace_count > 0 and body_end < len(content):
                if content[body_end] == '{':
                    brace_count += 1
                elif content[body_end] == '}':
                    brace_count -= 1
                body_end += 1
            
            loop_body = content[body_start:body_end]
            
            # Check if variable is modified in loop body
            # Look for assignments, increments, method calls that might modify
            modify_patterns = [
                rf'\b{var_name}\s*=',  # assignment
                rf'\b{var_name}\s*\+=',  # compound assignment
                rf'\b{var_name}\s*-=',
                rf'\b{var_name}\s*\*=',
                rf'\b{var_name}\s*/=',
                rf'\+\+{var_name}\b',  # pre-increment
                rf'{var_name}\+\+',    # post-increment
                rf'--{var_name}\b',    # pre-decrement
                rf'{var_name}--',      # post-decrement
                rf'{var_name}\s*\.\s*\w+\s*\(',  # method calls (might modify)
            ]
            
            is_modified = False
            for mod_pattern in modify_patterns:
                if re.search(mod_pattern, loop_body):
                    is_modified = True
                    break
            
            if not is_modified:
                fixes.append((match.start(), match.end(), 
                            f'for (const auto& {var_name} : {match.group(2)})'))
        
        # Apply fixes in reverse order
        for start, end, replacement in reversed(fixes):
            content = content[:start] + replacement + content[end:]
            self.fixes_applied += 1
            
        return content
    
    def fix_parameter_const(self, content):
        """Add const to function parameters that should be const references"""
        # Pattern to match function parameters that could be const ref
        # Matches: functionName(Type& param) or functionName(Type &param)
        pattern = re.compile(
            r'(\w+)\s*\(\s*([^)]*?)((?:std::|)(?:string|vector|list|map|set)\w*)\s*&\s*(\w+)([^)]*)\)'
        )
        
        fixes = []
        for match in pattern.finditer(content):
            func_name = match.group(1)
            before_type = match.group(2)
            type_name = match.group(3)
            param_name = match.group(4)
            after_param = match.group(5)
            
            # Skip if already const
            if 'const' in before_type or 'const' in match.group(0):
                continue
                
            # Look for the function body
            func_start = match.end()
            body_start = content.find('{', func_start)
            
            if body_start != -1:
                # Find matching closing brace
                brace_count = 1
                body_end = body_start + 1
                while brace_count > 0 and body_end < len(content):
                    if content[body_end] == '{':
                        brace_count += 1
                    elif content[body_end] == '}':
                        brace_count -= 1
                    body_end += 1
                
                func_body = content[body_start:body_end]
                
                # Check if parameter is modified
                modify_patterns = [
                    rf'\b{param_name}\s*=',
                    rf'\b{param_name}\s*\.',
                    rf'\b{param_name}\[',
                ]
                
                is_modified = False
                for mod_pattern in modify_patterns:
                    if re.search(mod_pattern, func_body):
                        is_modified = True
                        break
                
                if not is_modified:
                    new_params = f'{before_type}const {type_name}& {param_name}{after_param}'
                    fixes.append((match.start() + len(func_name) + 1, 
                                 match.end() - 1, new_params))
        
        # Apply fixes in reverse order
        for start, end, replacement in reversed(fixes):
            content = content[:start] + replacement + content[end:]
            self.fixes_applied += 1
            
        return content
    
    def process_file(self, filepath):
        """Process a single file to fix const warnings"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}")
            return False
        
        original_content = content
        
        # Apply different fix types
        content = self.fix_auto_reference(content)
        content = self.fix_parameter_const(content)
        
        if content != original_content:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"Fixed {filepath}")
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
        print("Usage: python3 fix_const_warnings.py <directory>")
        sys.exit(1)
    
    fixer = ConstReferenceFixer()
    
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
    print(f"Applied {fixer.fixes_applied} fixes")

if __name__ == "__main__":
    main()