#!/usr/bin/env python3
"""
Fix missing override specifiers in virtual functions
"""
import os
import re
import sys
from typing import Dict, List, Set, Tuple

class OverrideFixer:
    def __init__(self):
        self.class_hierarchy = {}  # class_name -> base_classes
        self.virtual_functions = {}  # class_name -> [(func_name, signature)]
        self.files_modified = 0
        
    def extract_class_info(self, content: str) -> Dict[str, List[str]]:
        """Extract class definitions and their base classes"""
        classes = {}
        
        # Match class definitions with inheritance
        class_pattern = r'class\s+(\w+)\s*(?::\s*(.+?))?\s*\{'
        
        for match in re.finditer(class_pattern, content, re.MULTILINE | re.DOTALL):
            class_name = match.group(1)
            inheritance = match.group(2) if match.group(2) else ""
            
            # Parse base classes
            base_classes = []
            if inheritance:
                # Remove access specifiers and extract class names
                for base in inheritance.split(','):
                    base = base.strip()
                    # Remove public/private/protected
                    base = re.sub(r'^(public|private|protected)\s+', '', base)
                    base_name = base.strip()
                    if base_name:
                        base_classes.append(base_name)
            
            classes[class_name] = base_classes
            
        return classes
    
    def extract_virtual_functions(self, content: str, class_name: str) -> List[Tuple[str, str]]:
        """Extract virtual function signatures from a class"""
        functions = []
        
        # Find class definition
        class_match = re.search(rf'class\s+{class_name}\s*(?::[^{{]+)?\s*{{', content)
        if not class_match:
            return functions
            
        # Extract class body
        brace_count = 1
        start_pos = class_match.end()
        pos = start_pos
        
        while pos < len(content) and brace_count > 0:
            if content[pos] == '{':
                brace_count += 1
            elif content[pos] == '}':
                brace_count -= 1
            pos += 1
            
        class_body = content[start_pos:pos-1]
        
        # Find virtual functions
        virtual_pattern = r'virtual\s+([^;{]+)\s*(?:=\s*0\s*)?;'
        
        for match in re.finditer(virtual_pattern, class_body):
            func_sig = match.group(1).strip()
            
            # Extract function name
            # Handle various return types and function signatures
            func_name_match = re.search(r'(\w+)\s*\(', func_sig)
            if func_name_match:
                func_name = func_name_match.group(1)
                functions.append((func_name, func_sig))
                
        return functions
    
    def needs_override(self, func_sig: str) -> bool:
        """Check if a function signature needs override keyword"""
        # Don't add override to:
        # 1. Pure virtual functions (= 0)
        # 2. Functions that already have override
        # 3. Destructors (handled separately)
        
        if '= 0' in func_sig:
            return False
        if 'override' in func_sig:
            return False
        if '~' in func_sig:  # Destructor
            return False
            
        return True
    
    def add_override_to_function(self, content: str, class_name: str, func_name: str, func_sig: str) -> str:
        """Add override specifier to a specific function"""
        # Build pattern to match the function
        # Need to be careful with whitespace and const
        
        # Escape special characters in function signature
        escaped_sig = re.escape(func_sig)
        
        # Replace escaped spaces with flexible whitespace
        escaped_sig = escaped_sig.replace(r'\ ', r'\s+')
        
        # Pattern to match virtual function
        pattern = rf'(virtual\s+{escaped_sig}\s*);'
        
        # Replace with override
        def replace_func(match):
            full_match = match.group(0)
            # Remove trailing semicolon, add override and semicolon
            return full_match[:-1] + ' override;'
            
        new_content = re.sub(pattern, replace_func, content)
        
        # If virtual not found, try without virtual keyword
        if new_content == content:
            pattern = rf'({escaped_sig}\s*);'
            new_content = re.sub(pattern, lambda m: m.group(0)[:-1] + ' override;', content)
            
        return new_content
    
    def fix_file(self, filepath: str) -> bool:
        """Fix missing override specifiers in a file"""
        with open(filepath, 'r') as f:
            content = f.read()
            
        original_content = content
        
        # First pass: fix obvious errors
        # Remove double override
        content = re.sub(r'override\s*=\s*default\s+override', 'override = default', content)
        content = re.sub(r'override\s+override', 'override', content)
        
        # Remove override from pure virtual functions
        content = re.sub(r'=\s*0\s*override', '= 0', content)
        
        # Extract class information
        classes = self.extract_class_info(content)
        
        # For each derived class, check virtual functions
        for class_name, base_classes in classes.items():
            if not base_classes:
                continue
                
            # Find virtual functions that might override base class methods
            # Look for virtual functions without override
            class_pattern = rf'class\s+{class_name}[^{{]*{{'
            class_match = re.search(class_pattern, content)
            
            if not class_match:
                continue
                
            # Find the class body
            brace_count = 1
            start_pos = class_match.end()
            pos = start_pos
            
            while pos < len(content) and brace_count > 0:
                if content[pos] == '{':
                    brace_count += 1
                elif content[pos] == '}':
                    brace_count -= 1
                pos += 1
                
            class_end = pos - 1
            
            # Look for virtual functions without override
            # Pattern: virtual ... function(...) ... ; (without override)
            virtual_pattern = r'virtual\s+([^;{]+?)(\s*)(;|\{)'
            
            # Process matches within class body
            for match in re.finditer(virtual_pattern, content[start_pos:class_end]):
                func_decl = match.group(1)
                
                # Skip if already has override
                if 'override' in func_decl:
                    continue
                    
                # Skip pure virtual
                if '= 0' in func_decl or '=0' in func_decl:
                    continue
                    
                # Skip destructors - they don't override
                if '~' in func_decl:
                    continue
                    
                # Add override
                absolute_start = start_pos + match.start()
                absolute_end = start_pos + match.end()
                
                # Insert override before semicolon or brace
                if match.group(3) == ';':
                    insert_pos = absolute_end - 1
                    content = content[:insert_pos] + ' override' + content[insert_pos:]
                elif match.group(3) == '{':
                    insert_pos = absolute_end - 1  
                    content = content[:insert_pos] + ' override ' + content[insert_pos:]
                    
        # Second pass: fix any const member functions
        # Pattern: function(...) const ; (without override)
        const_func_pattern = r'(\)\s*const)(\s*)(;|\{)(?!.*override)'
        
        # Only apply within class bodies
        classes_pattern = r'class\s+\w+[^{]*{[^}]+}'
        
        for class_match in re.finditer(classes_pattern, content, re.DOTALL):
            class_body = class_match.group(0)
            
            # Check if this class has inheritance
            if ':' not in class_body.split('{')[0]:
                continue
                
            # Find const functions without override
            for func_match in re.finditer(const_func_pattern, class_body):
                # Verify this is likely an overriding function
                func_start = max(0, func_match.start() - 100)
                func_context = class_body[func_start:func_match.end()]
                
                # Skip if it's likely a new function (has virtual keyword)
                if 'virtual' in func_context and 'override' not in func_context:
                    continue
                    
        # Fix destructor overrides (remove them)
        content = re.sub(r'(~\w+\s*\(\s*\)[^;{]*)\s+override', r'\1', content)
        
        if content != original_content:
            with open(filepath, 'w') as f:
                f.write(content)
            self.files_modified += 1
            return True
            
        return False
    
    def process_directory(self, directory: str):
        """Process all C++ files in directory"""
        for root, dirs, files in os.walk(directory):
            # Skip hidden directories
            dirs[:] = [d for d in dirs if not d.startswith('.')]
            
            for file in files:
                if file.endswith(('.h', '.hpp', '.cpp', '.cc')):
                    filepath = os.path.join(root, file)
                    try:
                        if self.fix_file(filepath):
                            print(f"Fixed: {filepath}")
                    except Exception as e:
                        print(f"Error processing {filepath}: {e}")

def main():
    fixer = OverrideFixer()
    
    os.chdir('/Users/jerry/lpzrobot_mac')
    
    components = ['selforg', 'ga_tools', 'opende', 'ode_robots']
    
    for component in components:
        print(f"\nProcessing {component}...")
        fixer.process_directory(component)
    
    print(f"\nTotal files modified: {fixer.files_modified}")

if __name__ == '__main__':
    main()