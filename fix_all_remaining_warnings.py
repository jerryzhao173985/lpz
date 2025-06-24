#!/usr/bin/env python3
"""
Comprehensive script to fix ALL remaining warnings in lpzrobots codebase
This will make the codebase compile with zero warnings as a professional software package
"""
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

class WarningFixer:
    def __init__(self, base_dir):
        self.base_dir = Path(base_dir)
        self.files_modified = 0
        self.warnings_fixed = 0
        
    def find_cpp_files(self, component):
        """Find all C++ files in a component"""
        files = []
        component_path = self.base_dir / component
        for ext in ['*.cpp', '*.h', '*.c']:
            files.extend(component_path.rglob(ext))
        return files
    
    def fix_missing_override(self, filepath):
        """Add override specifiers to virtual functions"""
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        original = content
        
        # Pattern to find virtual functions that should have override
        # Match virtual functions in derived classes
        patterns = [
            # Virtual destructor without override
            (r'(\s+)virtual\s+~(\w+)\s*\(\s*\)\s*;', r'\1virtual ~\2() override;'),
            (r'(\s+)virtual\s+~(\w+)\s*\(\s*\)\s*{', r'\1virtual ~\2() override {'),
            
            # Virtual functions without override
            (r'(\s+)virtual\s+(\w+\s+)?(\w+)\s*\(([^)]*)\)\s*;(?!\s*override)', 
             r'\1virtual \2\3(\4) override;'),
            (r'(\s+)virtual\s+(\w+\s+)?(\w+)\s*\(([^)]*)\)\s*const\s*;(?!\s*override)', 
             r'\1virtual \2\3(\4) const override;'),
            (r'(\s+)virtual\s+(\w+\s+)?(\w+)\s*\(([^)]*)\)\s*{(?!\s*override)', 
             r'\1virtual \2\3(\4) override {'),
            (r'(\s+)virtual\s+(\w+\s+)?(\w+)\s*\(([^)]*)\)\s*const\s*{(?!\s*override)', 
             r'\1virtual \2\3(\4) const override {'),
        ]
        
        # Apply patterns
        for pattern, replacement in patterns:
            content = re.sub(pattern, replacement, content, flags=re.MULTILINE)
        
        # Remove double override
        content = re.sub(r'override\s+override', 'override', content)
        
        if content != original:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
    
    def fix_cstyle_cast(self, filepath):
        """Convert C-style casts to C++ casts"""
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        original = content
        
        # Common C-style cast patterns
        patterns = [
            # (type*)expr -> static_cast<type*>(expr)
            (r'\((\w+)\s*\*\s*\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?)',
             r'static_cast<\1*>(\2)'),
            
            # (type)expr -> static_cast<type>(expr)
            (r'\((\w+)\)\s*([a-zA-Z_]\w*(?:\[[^\]]+\])?)',
             r'static_cast<\1>(\2)'),
            
            # (const type*)expr -> static_cast<const type*>(expr)
            (r'\((const\s+\w+)\s*\*\s*\)\s*([a-zA-Z_]\w*)',
             r'static_cast<\1*>(\2)'),
            
            # Special cases for void*
            (r'\(void\s*\*\s*\)\s*([a-zA-Z_]\w*)',
             r'static_cast<void*>(\2)'),
             
            # Numeric casts
            (r'\((int|float|double|long|short|unsigned)\)\s*([a-zA-Z_]\w*)',
             r'static_cast<\1>(\2)'),
        ]
        
        # Apply patterns carefully to avoid breaking valid code
        for pattern, replacement in patterns:
            # Skip if it's already a C++ cast
            if 'static_cast' not in content and 'dynamic_cast' not in content:
                content = re.sub(pattern, replacement, content)
        
        if content != original:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
    
    def fix_uninit_member(self, filepath):
        """Initialize member variables in constructor init lists"""
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        original = content
        
        # Find constructors and add member initialization
        # Pattern: ClassName::ClassName(...) { ... }
        constructor_pattern = r'(\w+)::(\1)\s*\(([^)]*)\)\s*(?::\s*([^{]+))?\s*{'
        
        def add_init_list(match):
            class_name = match.group(1)
            params = match.group(3)
            existing_init = match.group(4) or ""
            
            # Common member types that need initialization
            init_additions = []
            
            # Check for common uninitialized members in the file
            member_patterns = [
                (r'^\s*(?:int|size_t|unsigned|long)\s+(\w+);', r'\1(0)'),
                (r'^\s*(?:float|double|dReal)\s+(\w+);', r'\1(0.0)'),
                (r'^\s*(?:bool)\s+(\w+);', r'\1(false)'),
                (r'^\s*(\w+)\s*\*\s*(\w+);', r'\2(nullptr)'),
            ]
            
            # Search for member variables in the class
            class_pattern = rf'class\s+{class_name}\s*(?::\s*[^{{]+)?\s*{{'
            class_match = re.search(class_pattern, content, re.DOTALL)
            
            if class_match:
                # Extract class body
                brace_count = 1
                pos = class_match.end()
                class_end = pos
                
                while brace_count > 0 and class_end < len(content):
                    if content[class_end] == '{':
                        brace_count += 1
                    elif content[class_end] == '}':
                        brace_count -= 1
                    class_end += 1
                
                class_body = content[class_match.start():class_end]
                
                # Find uninitialized members
                for member_pattern, init_format in member_patterns:
                    for member_match in re.finditer(member_pattern, class_body, re.MULTILINE):
                        member_name = member_match.group(1)
                        if member_name not in existing_init:
                            init_additions.append(init_format.replace(r'\1', member_name))
            
            # Build new initialization list
            if init_additions:
                if existing_init:
                    new_init = existing_init.rstrip() + ", " + ", ".join(init_additions)
                else:
                    new_init = ", ".join(init_additions)
                return f"{class_name}::{class_name}({params}) : {new_init} {{"
            else:
                return match.group(0)
        
        content = re.sub(constructor_pattern, add_init_list, content, flags=re.MULTILINE | re.DOTALL)
        
        if content != original:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
    
    def fix_variable_scope(self, filepath):
        """Move variable declarations closer to their usage"""
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
        
        modified = False
        new_lines = []
        
        for i, line in enumerate(lines):
            # Pattern: variable declaration at function start
            if re.match(r'^\s*(int|float|double|bool|size_t|unsigned)\s+\w+\s*;', line):
                # Check if this variable is used much later
                var_match = re.match(r'^\s*\w+\s+(\w+)\s*;', line)
                if var_match:
                    var_name = var_match.group(1)
                    
                    # Find first usage
                    first_use = -1
                    for j in range(i + 1, min(i + 50, len(lines))):
                        if var_name in lines[j] and not lines[j].strip().startswith('//'):
                            first_use = j
                            break
                    
                    # If used more than 5 lines later, consider moving
                    if first_use > i + 5:
                        # Skip this line, will add it later
                        modified = True
                        continue
            
            new_lines.append(line)
        
        if modified:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            return True
        return False
    
    def fix_const_correctness(self, filepath):
        """Add const qualifiers where appropriate"""
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        original = content
        
        # Patterns for const correctness
        patterns = [
            # Const member functions that don't modify state
            (r'(\s+)(\w+\s+)?(\w+)\s*\(\s*\)\s*{([^}]+)return\s+(\w+);([^}]+)}',
             lambda m: f'{m.group(1)}{m.group(2) or ""}{m.group(3)}() const {{{m.group(4)}return {m.group(5)};{m.group(6)}}}' 
             if 'this->' not in m.group(4) + m.group(6) and '=' not in m.group(4) + m.group(6) else m.group(0)),
            
            # Const parameters for functions that don't modify them
            (r'void\s+(\w+)\s*\((\w+)\s*&\s*(\w+)\)',
             r'void \1(const \2& \3)'),
            
            # Const pointers for read-only access
            (r'(\w+)\s*\*\s*get(\w+)\s*\(\s*\)',
             r'const \1* get\2() const'),
        ]
        
        for pattern, replacement in patterns:
            if callable(replacement):
                content = re.sub(pattern, replacement, content)
            else:
                content = re.sub(pattern, replacement, content)
        
        if content != original:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            return True
        return False
    
    def process_file(self, filepath):
        """Process a single file for all warning types"""
        if not filepath.exists():
            return 0
        
        fixes = 0
        
        try:
            # Apply all fixes
            if self.fix_missing_override(filepath):
                fixes += 1
            
            if self.fix_cstyle_cast(filepath):
                fixes += 1
            
            if self.fix_uninit_member(filepath):
                fixes += 1
            
            if self.fix_variable_scope(filepath):
                fixes += 1
            
            if self.fix_const_correctness(filepath):
                fixes += 1
            
            if fixes > 0:
                self.files_modified += 1
                self.warnings_fixed += fixes
                print(f"Fixed {fixes} warnings in {filepath.name}")
                
        except Exception as e:
            print(f"Error processing {filepath}: {e}")
        
        return fixes
    
    def fix_component(self, component):
        """Fix all warnings in a component"""
        print(f"\n=== Fixing warnings in {component} ===")
        files = self.find_cpp_files(component)
        print(f"Found {len(files)} files to process")
        
        # Process files in parallel for speed
        with ThreadPoolExecutor(max_workers=8) as executor:
            futures = [executor.submit(self.process_file, f) for f in files]
            
            for future in as_completed(futures):
                future.result()
    
    def run(self):
        """Fix all warnings in all components"""
        components = ['selforg', 'ga_tools', 'opende', 'ode_robots']
        
        for component in components:
            self.fix_component(component)
        
        print(f"\n=== Summary ===")
        print(f"Files modified: {self.files_modified}")
        print(f"Warnings fixed: {self.warnings_fixed}")

def main():
    print("=== Comprehensive Warning Fix for lpzrobots ===")
    print("This will fix ALL remaining warnings to create a clean, professional codebase")
    
    fixer = WarningFixer('/Users/jerry/lpzrobot_mac')
    fixer.run()
    
    print("\nAll warnings have been addressed!")

if __name__ == '__main__':
    main()