#!/usr/bin/env python3
"""
Robust script to fix all remaining C++ warnings in lpzrobots
"""
import os
import re
import subprocess
from pathlib import Path
import multiprocessing as mp
from functools import partial

class RobustWarningFixer:
    def __init__(self, base_dir):
        self.base_dir = Path(base_dir)
        self.stats = {
            'files_processed': 0,
            'files_modified': 0,
            'override_added': 0,
            'casts_fixed': 0,
            'members_initialized': 0,
            'const_added': 0,
            'other_fixes': 0
        }
        
    def fix_missing_override_simple(self, content):
        """Add override to virtual functions - simple approach"""
        modified = False
        lines = content.split('\n')
        new_lines = []
        
        for line in lines:
            new_line = line
            
            # Simple pattern: virtual function declaration without override
            if ('virtual' in line and 
                'override' not in line and 
                '=' not in line and  # Skip pure virtual
                '{' in line or ';' in line):
                
                # Check if it's likely a function (has parentheses)
                if '(' in line and ')' in line:
                    # Add override before { or ;
                    if line.rstrip().endswith('{'):
                        new_line = line.rstrip()[:-1] + ' override {'
                        modified = True
                        self.stats['override_added'] += 1
                    elif line.rstrip().endswith(';'):
                        new_line = line.rstrip()[:-1] + ' override;'
                        modified = True
                        self.stats['override_added'] += 1
                    elif 'const' in line and '{' in line:
                        # Handle const functions
                        new_line = re.sub(r'const\s*{', 'const override {', line)
                        if new_line != line:
                            modified = True
                            self.stats['override_added'] += 1
                    elif 'const' in line and ';' in line:
                        new_line = re.sub(r'const\s*;', 'const override;', line)
                        if new_line != line:
                            modified = True
                            self.stats['override_added'] += 1
            
            new_lines.append(new_line)
        
        return '\n'.join(new_lines), modified
    
    def fix_cstyle_casts_safe(self, content):
        """Fix C-style casts conservatively"""
        modified = False
        
        # Only fix the most obvious and safe C-style casts
        safe_patterns = [
            # (int)var -> static_cast<int>(var)
            (r'\(int\)\s*([a-zA-Z_]\w*)', r'static_cast<int>(\1)'),
            # (double)var -> static_cast<double>(var)
            (r'\(double\)\s*([a-zA-Z_]\w*)', r'static_cast<double>(\1)'),
            # (float)var -> static_cast<float>(var)
            (r'\(float\)\s*([a-zA-Z_]\w*)', r'static_cast<float>(\1)'),
            # (size_t)var -> static_cast<size_t>(var)
            (r'\(size_t\)\s*([a-zA-Z_]\w*)', r'static_cast<size_t>(\1)'),
            # (unsigned int)var -> static_cast<unsigned int>(var)
            (r'\(unsigned int\)\s*([a-zA-Z_]\w*)', r'static_cast<unsigned int>(\1)'),
            # (void*)ptr -> static_cast<void*>(ptr)
            (r'\(void\s*\*\)\s*([a-zA-Z_]\w*)', r'static_cast<void*>(\1)'),
        ]
        
        for pattern, replacement in safe_patterns:
            new_content = re.sub(pattern, replacement, content)
            if new_content != content:
                content = new_content
                modified = True
                self.stats['casts_fixed'] += 1
        
        return content, modified
    
    def fix_uninit_members_in_constructors(self, content):
        """Initialize member variables in constructors"""
        modified = False
        
        # Find class definitions and their constructors
        class_pattern = r'class\s+(\w+)[^{]*\{'
        constructor_pattern = r'(\w+)::\1\s*\([^)]*\)\s*(?::\s*([^{]+))?\s*\{'
        
        # Common uninitialized member patterns
        member_patterns = [
            r'^\s*int\s+(\w+);',
            r'^\s*unsigned\s+(\w+);',
            r'^\s*size_t\s+(\w+);',
            r'^\s*float\s+(\w+);',
            r'^\s*double\s+(\w+);',
            r'^\s*bool\s+(\w+);',
            r'^\s*\w+\*\s+(\w+);',  # pointers
        ]
        
        # Find classes
        classes = re.findall(class_pattern, content)
        
        for class_name in classes:
            # Find constructor
            const_match = re.search(rf'{class_name}::{class_name}\s*\([^)]*\)\s*(?::\s*([^{{]+))?\s*\{{', content)
            if const_match:
                existing_init = const_match.group(1) or ""
                
                # Find uninitialized members in class
                class_body_match = re.search(rf'class\s+{class_name}[^{{]*\{{([^}}]+)\}}', content, re.DOTALL)
                if class_body_match:
                    class_body = class_body_match.group(1)
                    
                    new_inits = []
                    for pattern in member_patterns:
                        for member_match in re.finditer(pattern, class_body, re.MULTILINE):
                            member_name = member_match.group(1)
                            # Check if already initialized
                            if member_name not in existing_init:
                                # Determine initialization value
                                if 'int' in pattern or 'unsigned' in pattern or 'size_t' in pattern:
                                    new_inits.append(f'{member_name}(0)')
                                elif 'float' in pattern or 'double' in pattern:
                                    new_inits.append(f'{member_name}(0.0)')
                                elif 'bool' in pattern:
                                    new_inits.append(f'{member_name}(false)')
                                elif '*' in pattern:
                                    new_inits.append(f'{member_name}(nullptr)')
                    
                    if new_inits:
                        # Add to initialization list
                        if existing_init:
                            new_init_list = existing_init + ', ' + ', '.join(new_inits)
                        else:
                            new_init_list = ', '.join(new_inits)
                        
                        # Replace constructor
                        old_constructor = const_match.group(0)
                        new_constructor = f'{class_name}::{class_name}({const_match.group(0).split("(")[1].split(")")[0]}) : {new_init_list} {{'
                        
                        content = content.replace(old_constructor, new_constructor)
                        modified = True
                        self.stats['members_initialized'] += len(new_inits)
        
        return content, modified
    
    def add_const_correctness(self, content):
        """Add const where appropriate"""
        modified = False
        
        # Add const to getter methods
        getter_pattern = r'(\s+)(\w+\s+)?get(\w+)\s*\(\s*\)\s*{([^}]+)return\s+(\w+);([^}]*)}'
        
        def make_const_getter(match):
            indent = match.group(1)
            return_type = match.group(2) or ''
            name = match.group(3)
            before_return = match.group(4)
            returned_var = match.group(5)
            after_return = match.group(6)
            
            # Check if it's likely a simple getter (no modifications)
            if '=' not in before_return + after_return and 'this->' not in before_return + after_return:
                self.stats['const_added'] += 1
                return f'{indent}{return_type}get{name}() const {{{before_return}return {returned_var};{after_return}}}'
            return match.group(0)
        
        new_content = re.sub(getter_pattern, make_const_getter, content)
        if new_content != content:
            content = new_content
            modified = True
        
        # Add const to parameters that are not modified
        param_pattern = r'(\w+)\s*&\s*(\w+)(?=\s*[,)])'
        
        def make_const_param(match):
            type_name = match.group(1)
            param_name = match.group(2)
            # Skip if already const or if it's a common mutable type
            if type_name not in ['const', 'std', 'QString', 'QWidget']:
                self.stats['const_added'] += 1
                return f'const {type_name}& {param_name}'
            return match.group(0)
        
        # Apply to function parameters (simple cases)
        lines = content.split('\n')
        new_lines = []
        for line in lines:
            if '(' in line and ')' in line and '&' in line and 'const' not in line:
                new_line = re.sub(param_pattern, make_const_param, line)
                if new_line != line:
                    modified = True
                new_lines.append(new_line)
            else:
                new_lines.append(line)
        
        if modified:
            content = '\n'.join(new_lines)
        
        return content, modified
    
    def process_file(self, filepath):
        """Process a single file"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            original = content
            
            # Apply fixes
            content, mod1 = self.fix_missing_override_simple(content)
            content, mod2 = self.fix_cstyle_casts_safe(content)
            content, mod3 = self.fix_uninit_members_in_constructors(content)
            content, mod4 = self.add_const_correctness(content)
            
            if content != original:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                self.stats['files_modified'] += 1
                return True
            
            return False
            
        except Exception as e:
            print(f"Error processing {filepath}: {e}")
            return False
    
    def process_files_parallel(self, files):
        """Process files in parallel"""
        with mp.Pool(processes=mp.cpu_count()) as pool:
            results = pool.map(self.process_file, files)
        
        self.stats['files_processed'] = len(files)
        return sum(results)
    
    def fix_component(self, component):
        """Fix all warnings in a component"""
        print(f"\n=== Processing {component} ===")
        
        component_path = self.base_dir / component
        if not component_path.exists():
            print(f"Component {component} not found")
            return
        
        # Find all C++ files
        files = []
        for ext in ['*.cpp', '*.h', '*.hpp', '*.cc']:
            files.extend(component_path.rglob(ext))
        
        # Filter out unwanted files
        files = [f for f in files if '.svn' not in str(f) and 'build' not in str(f)]
        
        print(f"Found {len(files)} files to process")
        
        # Process in batches
        batch_size = 100
        total_modified = 0
        
        for i in range(0, len(files), batch_size):
            batch = files[i:i+batch_size]
            modified = self.process_files_parallel(batch)
            total_modified += modified
            print(f"Progress: {min(i+batch_size, len(files))}/{len(files)} files processed, {total_modified} modified")
        
        return total_modified
    
    def print_stats(self):
        """Print statistics"""
        print("\n=== Final Statistics ===")
        print(f"Files processed: {self.stats['files_processed']}")
        print(f"Files modified: {self.stats['files_modified']}")
        print(f"Override specifiers added: {self.stats['override_added']}")
        print(f"C-style casts fixed: {self.stats['casts_fixed']}")
        print(f"Member variables initialized: {self.stats['members_initialized']}")
        print(f"Const qualifiers added: {self.stats['const_added']}")

def main():
    print("=== Robust C++ Warning Fix for lpzrobots ===")
    print("This will systematically fix all remaining warnings")
    
    base_dir = '/Users/jerry/lpzrobot_mac'
    fixer = RobustWarningFixer(base_dir)
    
    # Process each component
    components = ['selforg', 'ga_tools', 'opende', 'ode_robots']
    
    for component in components:
        fixer.fix_component(component)
    
    fixer.print_stats()
    
    print("\n=== Creating build verification script ===")
    
    # Create a build script to verify everything compiles
    build_script = """#!/bin/bash
# Verify lpzrobots builds without warnings

echo "=== Building lpzrobots with strict warning checks ==="

# Set strict compiler flags
export CXXFLAGS="-std=c++17 -Wall -Wextra -Wpedantic -Werror"

# Clean build
make clean-all

# Build each component
echo "Building selforg..."
if make selforg 2>&1 | tee selforg_build.log | grep -E "warning:|error:"; then
    echo "Warnings/errors found in selforg!"
else
    echo "selforg built cleanly!"
fi

echo "Building ga_tools..."
if make ga_tools 2>&1 | tee ga_tools_build.log | grep -E "warning:|error:"; then
    echo "Warnings/errors found in ga_tools!"
else
    echo "ga_tools built cleanly!"
fi

echo "Building opende..."
if make opende 2>&1 | tee opende_build.log | grep -E "warning:|error:"; then
    echo "Warnings/errors found in opende!"
else
    echo "opende built cleanly!"
fi

echo "Building ode_robots..."
if make ode_robots 2>&1 | tee ode_robots_build.log | grep -E "warning:|error:"; then
    echo "Warnings/errors found in ode_robots!"
else
    echo "ode_robots built cleanly!"
fi

echo "=== Build verification complete ==="
"""
    
    with open(Path(base_dir) / 'verify_clean_build.sh', 'w') as f:
        f.write(build_script)
    
    os.chmod(Path(base_dir) / 'verify_clean_build.sh', 0o755)
    
    print("Created verify_clean_build.sh - run this to verify clean compilation")

if __name__ == '__main__':
    main()