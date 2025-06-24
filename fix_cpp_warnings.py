#!/usr/bin/env python3
"""
Comprehensive script to fix C++ standard warnings in LPZRobots codebase
"""
import os
import re
import sys

class CppWarningFixer:
    def __init__(self):
        self.files_processed = 0
        self.files_modified = 0
        
    def fix_file(self, filepath):
        """Fix C++ warnings in a single file"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except:
            return False
            
        original_content = content
        
        # Fix 1: Add override to virtual destructors
        content = self.fix_virtual_destructors(content)
        
        # Fix 2: Replace C-style casts with C++ casts
        content = self.fix_c_style_casts(content)
        
        # Fix 3: Fix uninitialized member variables
        content = self.fix_uninitialized_members(content)
        
        # Fix 4: Add explicit to single-parameter constructors
        content = self.fix_explicit_constructors(content)
        
        # Fix 5: Fix const correctness
        content = self.fix_const_correctness(content)
        
        # Fix 6: Replace NULL with nullptr
        content = self.fix_null_to_nullptr(content)
        
        # Fix 7: Fix old-style headers
        content = self.fix_old_headers(content)
        
        # Fix 8: Fix variable scope issues
        content = self.fix_variable_scope(content)
        
        # Fix 9: Fix pass-by-value to pass-by-const-ref
        content = self.fix_pass_by_value(content)
        
        # Fix 10: Fix postfix increment/decrement
        content = self.fix_postfix_operators(content)
        
        if content != original_content:
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                self.files_modified += 1
                return True
            except:
                return False
        return False
    
    def fix_virtual_destructors(self, content):
        """Add override to virtual destructors"""
        # Pattern: virtual ~ClassName(); without override
        pattern = r'(\s*virtual\s+~\w+\s*\(\s*\)\s*);(?!\s*override)'
        
        def add_override(match):
            return match.group(1) + ' override;'
        
        # Only in derived classes (check for inheritance)
        if ': public' in content or ': private' in content or ': protected' in content:
            content = re.sub(pattern, add_override, content)
        
        return content
    
    def fix_c_style_casts(self, content):
        """Replace C-style casts with C++ casts"""
        # Common patterns
        patterns = [
            # (int)var -> static_cast<int>(var)
            (r'\(int\)\s*([a-zA-Z_]\w*(?:\.\w+)*)', r'static_cast<int>(\1)'),
            (r'\(double\)\s*([a-zA-Z_]\w*(?:\.\w+)*)', r'static_cast<double>(\1)'),
            (r'\(float\)\s*([a-zA-Z_]\w*(?:\.\w+)*)', r'static_cast<float>(\1)'),
            (r'\(size_t\)\s*([a-zA-Z_]\w*(?:\.\w+)*)', r'static_cast<size_t>(\1)'),
            (r'\(unsigned int\)\s*([a-zA-Z_]\w*(?:\.\w+)*)', r'static_cast<unsigned int>(\1)'),
            (r'\(long\)\s*([a-zA-Z_]\w*(?:\.\w+)*)', r'static_cast<long>(\1)'),
            (r'\(char\)\s*([a-zA-Z_]\w*(?:\.\w+)*)', r'static_cast<char>(\1)'),
            # (Type*)ptr -> static_cast<Type*>(ptr)
            (r'\((\w+)\s*\*\s*\)\s*([a-zA-Z_]\w*)', r'static_cast<\1*>(\2)'),
            # (const Type*)ptr -> static_cast<const Type*>(ptr)
            (r'\(const\s+(\w+)\s*\*\s*\)\s*([a-zA-Z_]\w*)', r'static_cast<const \1*>(\2)'),
        ]
        
        for pattern, replacement in patterns:
            content = re.sub(pattern, replacement, content)
        
        return content
    
    def fix_uninitialized_members(self, content):
        """Fix uninitialized member variables"""
        # Find class member declarations
        class_pattern = r'class\s+(\w+)[^{]*\{([^}]+)\}'
        
        def fix_class_members(match):
            class_name = match.group(1)
            class_body = match.group(2)
            
            # Fix uninitialized primitives
            replacements = [
                (r'(\s+)(int|unsigned int|size_t|long|unsigned long)\s+(\w+);', r'\1\2 \3 = 0;'),
                (r'(\s+)(float|double)\s+(\w+);', r'\1\2 \3 = 0.0;'),
                (r'(\s+)(bool)\s+(\w+);', r'\1\2 \3 = false;'),
                (r'(\s+)(\w+)\s*\*\s+(\w+);', r'\1\2* \3 = nullptr;'),
            ]
            
            for pattern, replacement in replacements:
                # Only fix if not already initialized
                class_body = re.sub(pattern + r'(?!\s*=)', replacement, class_body)
            
            return f'class {class_name}{match.group(0)[len(class_name)+5:len(match.group(1))+5]}{{{class_body}}}'
        
        content = re.sub(class_pattern, fix_class_members, content, flags=re.DOTALL)
        return content
    
    def fix_explicit_constructors(self, content):
        """Add explicit to single-parameter constructors"""
        # Pattern: Constructor with single parameter (not copy/move constructor)
        pattern = r'(\s+)(\w+)\s*\(\s*(?:const\s+)?(\w+)(?:\s*[&*])?\s+\w+\s*\)\s*(?:;|{)'
        
        def check_and_add_explicit(match):
            indent = match.group(1)
            ctor_name = match.group(2)
            param_type = match.group(3)
            
            # Skip copy/move constructors
            if param_type == ctor_name:
                return match.group(0)
            
            # Skip if already has explicit
            if 'explicit' in match.group(0):
                return match.group(0)
            
            # Add explicit
            return indent + 'explicit ' + match.group(0)[len(indent):]
        
        return re.sub(pattern, check_and_add_explicit, content)
    
    def fix_const_correctness(self, content):
        """Fix const correctness issues"""
        # Add const to getter methods
        getter_pattern = r'(\s+(?:virtual\s+)?)((?:int|double|float|bool|size_t|\w+(?:\s*[*&])?)\s+get\w+\s*\(\s*\)\s*)(?:override\s*)?;'
        
        def add_const_to_getter(match):
            prefix = match.group(1)
            getter = match.group(2)
            override = ' override' if 'override' in match.group(0) else ''
            
            if 'const' not in getter:
                return prefix + getter.rstrip() + ' const' + override + ';'
            return match.group(0)
        
        content = re.sub(getter_pattern, add_const_to_getter, content)
        
        # Add const to parameters that aren't modified
        param_pattern = r'(\w+)\s+(\w+)\s*\(\s*((?:int|double|float|bool|std::string)\s+)(\w+)\s*\)'
        
        def add_const_to_param(match):
            return_type = match.group(1)
            func_name = match.group(2)
            param_type = match.group(3)
            param_name = match.group(4)
            
            # Skip if already const
            if 'const' in param_type:
                return match.group(0)
            
            # Add const for string parameters
            if 'std::string' in param_type:
                return f'{return_type} {func_name}(const {param_type}&{param_name})'
            
            return match.group(0)
        
        content = re.sub(param_pattern, add_const_to_param, content)
        
        return content
    
    def fix_null_to_nullptr(self, content):
        """Replace NULL with nullptr"""
        # Replace NULL with nullptr
        content = re.sub(r'\bNULL\b', 'nullptr', content)
        
        # Replace 0 with nullptr in pointer contexts
        content = re.sub(r'=\s*0\s*;(\s*//.*pointer|.*\*)', r'= nullptr;\1', content)
        content = re.sub(r'==\s*0\s*\)', r'== nullptr)', content)
        content = re.sub(r'!=\s*0\s*\)', r'!= nullptr)', content)
        
        return content
    
    def fix_old_headers(self, content):
        """Replace old C headers with C++ headers"""
        header_map = {
            '<stdio.h>': '<cstdio>',
            '<stdlib.h>': '<cstdlib>',
            '<string.h>': '<cstring>',
            '<math.h>': '<cmath>',
            '<time.h>': '<ctime>',
            '<assert.h>': '<cassert>',
            '<limits.h>': '<climits>',
            '<float.h>': '<cfloat>',
            '<stddef.h>': '<cstddef>',
            '<signal.h>': '<csignal>',
        }
        
        for old_header, new_header in header_map.items():
            content = content.replace(f'#include {old_header}', f'#include {new_header}')
        
        return content
    
    def fix_variable_scope(self, content):
        """Fix variable scope issues - declare variables closer to use"""
        # This is complex and requires semantic analysis
        # For now, just fix simple cases like loop variables
        
        # Fix: for(int i = 0; ...) instead of int i; ... for(i = 0; ...)
        pattern = r'int\s+(\w+)\s*;\s*\n\s*for\s*\(\s*\1\s*=\s*0'
        replacement = r'for(int \1 = 0'
        content = re.sub(pattern, replacement, content)
        
        return content
    
    def fix_pass_by_value(self, content):
        """Fix pass-by-value for large objects"""
        # Fix std::string, std::vector, etc. passed by value
        patterns = [
            (r'(\w+)\s+(\w+)\s*\(\s*(std::string)\s+(\w+)', r'\1 \2(const \3& \4'),
            (r'(\w+)\s+(\w+)\s*\(\s*(std::vector<[^>]+>)\s+(\w+)', r'\1 \2(const \3& \4'),
            (r'(\w+)\s+(\w+)\s*\(\s*(std::map<[^>]+>)\s+(\w+)', r'\1 \2(const \3& \4'),
        ]
        
        for pattern, replacement in patterns:
            content = re.sub(pattern, replacement, content)
        
        return content
    
    def fix_postfix_operators(self, content):
        """Replace postfix ++ and -- with prefix where the result isn't used"""
        # Fix: i++ to ++i in for loops
        content = re.sub(r'for\s*\([^;]+;[^;]+;\s*(\w+)\+\+\s*\)', r'for(...; ++\1)', content)
        content = re.sub(r'for\s*\([^;]+;[^;]+;\s*(\w+)--\s*\)', r'for(...; --\1)', content)
        
        # Fix standalone postfix operations
        content = re.sub(r'^(\s*)(\w+)\+\+;', r'\1++\2;', content, flags=re.MULTILINE)
        content = re.sub(r'^(\s*)(\w+)--;', r'\1--\2;', content, flags=re.MULTILINE)
        
        return content
    
    def process_directory(self, directory):
        """Process all C++ files in a directory"""
        for root, dirs, files in os.walk(directory):
            # Skip hidden directories and build directories
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['build', 'dist', 'backup']]
            
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp', '.cc')):
                    filepath = os.path.join(root, file)
                    self.files_processed += 1
                    
                    if self.fix_file(filepath):
                        print(f"Fixed: {filepath}")
                    
                    if self.files_processed % 100 == 0:
                        print(f"Processed {self.files_processed} files...")
    
    def run(self, directories):
        """Run the fixer on multiple directories"""
        for directory in directories:
            if os.path.exists(directory):
                print(f"\nProcessing {directory}...")
                self.process_directory(directory)
            else:
                print(f"Warning: {directory} not found")
        
        print(f"\nTotal files processed: {self.files_processed}")
        print(f"Total files modified: {self.files_modified}")

def main():
    fixer = CppWarningFixer()
    
    # Process all major components
    directories = [
        'ga_tools',
        'selforg',
        'ode_robots',
        'opende',
        'guilogger',
        'matrixviz',
        'configurator'
    ]
    
    fixer.run(directories)

if __name__ == '__main__':
    main()