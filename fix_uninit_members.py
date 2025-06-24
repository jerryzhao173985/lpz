#!/usr/bin/env python3
"""
Fix uninitialized member variables in constructors
"""
import os
import re
import sys

class UninitMemberFixer:
    def __init__(self):
        self.files_modified = 0
        self.members_initialized = 0
        
    def extract_class_members(self, class_body: str) -> list:
        """Extract member variables from class body"""
        members = []
        
        # Skip method bodies
        in_method = False
        brace_count = 0
        lines = class_body.split('\n')
        
        for line in lines:
            # Track braces to skip method bodies
            if '{' in line:
                in_method = True
                brace_count += line.count('{')
            if '}' in line:
                brace_count -= line.count('}')
                if brace_count == 0:
                    in_method = False
                    
            if in_method:
                continue
                
            # Skip if line contains certain keywords
            skip_keywords = ['static', 'const static', 'typedef', 'using', 'friend', 'public:', 'private:', 'protected:', 'virtual', 'explicit', 'operator']
            if any(keyword in line for keyword in skip_keywords):
                continue
                
            # Match member variable declarations
            # Type name; or Type* name; or Type name[size];
            member_pattern = r'^\s*(?:mutable\s+)?([a-zA-Z_][\w:]*(?:\s*[*&])*)\s+([a-zA-Z_]\w*)(?:\[[^\]]*\])?\s*;'
            match = re.match(member_pattern, line)
            
            if match:
                member_type = match.group(1).strip()
                member_name = match.group(2).strip()
                
                # Skip certain types that don't need initialization
                skip_types = ['std::string', 'std::vector', 'std::map', 'std::set', 'std::list']
                if not any(skip_type in member_type for skip_type in skip_types):
                    members.append((member_type, member_name))
                    
        return members
    
    def get_initialization_value(self, member_type: str, member_name: str) -> str:
        """Get appropriate initialization value for a member type"""
        # Pointer types
        if '*' in member_type:
            return 'nullptr'
            
        # Reference types can't be default initialized
        if '&' in member_type:
            return None
            
        # Numeric types
        numeric_types = ['int', 'unsigned', 'long', 'short', 'char', 'float', 'double', 'size_t', 'bool']
        if any(ntype in member_type for ntype in numeric_types):
            if 'bool' in member_type:
                return 'false'
            return '0'
            
        # Enums - default construct
        if member_type.startswith('enum'):
            return ''
            
        # Other types - default construct
        return ''
    
    def fix_constructor_initialization(self, content: str, class_name: str, members: list) -> str:
        """Fix constructor initialization lists"""
        # Find constructors
        constructor_pattern = rf'{class_name}\s*\([^)]*\)\s*(?::\s*([^{{]+))?\s*{{'
        
        def fix_constructor(match):
            full_match = match.group(0)
            init_list = match.group(1) if match.group(1) else ""
            
            # Parse existing initializers
            existing_inits = set()
            if init_list:
                # Extract initialized members
                init_items = re.split(r',(?![^()]*\))', init_list)
                for item in init_items:
                    # Extract member name from initializer
                    init_match = re.match(r'\s*(\w+)\s*\(', item)
                    if init_match:
                        existing_inits.add(init_match.group(1))
            
            # Determine which members need initialization
            new_inits = []
            for member_type, member_name in members:
                if member_name not in existing_inits:
                    init_value = self.get_initialization_value(member_type, member_name)
                    if init_value is not None:
                        new_inits.append(f"{member_name}({init_value})")
                        self.members_initialized += 1
            
            # Combine initializers
            if new_inits:
                if init_list:
                    # Add to existing list
                    all_inits = init_list.strip() + ', ' + ', '.join(new_inits)
                else:
                    # Create new list
                    all_inits = ', '.join(new_inits)
                    
                # Reconstruct constructor
                return f"{class_name}{match.group(0)[len(class_name):match.start(1)-match.start()]} : {all_inits} {{"
            
            return full_match
        
        # Apply fixes
        new_content = re.sub(constructor_pattern, fix_constructor, content, flags=re.MULTILINE | re.DOTALL)
        
        return new_content
    
    def fix_file(self, filepath: str) -> bool:
        """Fix uninitialized members in a file"""
        try:
            with open(filepath, 'r') as f:
                content = f.read()
                
            original_content = content
            
            # Find all class definitions
            class_pattern = r'class\s+(\w+)(?:\s*:[^{]+)?\s*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}'
            
            for class_match in re.finditer(class_pattern, content, re.MULTILINE | re.DOTALL):
                class_name = class_match.group(1)
                class_body = class_match.group(2)
                
                # Extract members
                members = self.extract_class_members(class_body)
                
                if members:
                    # Fix constructor initialization
                    content = self.fix_constructor_initialization(content, class_name, members)
            
            # Also fix struct constructors
            struct_pattern = r'struct\s+(\w+)(?:\s*:[^{]+)?\s*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}'
            
            for struct_match in re.finditer(struct_pattern, content, re.MULTILINE | re.DOTALL):
                struct_name = struct_match.group(1)
                struct_body = struct_match.group(2)
                
                # Extract members
                members = self.extract_class_members(struct_body)
                
                if members:
                    # Fix constructor initialization
                    content = self.fix_constructor_initialization(content, struct_name, members)
            
            # Additional fix: Add initialization to member declarations (C++11 style)
            # int m_count; -> int m_count = 0;
            member_decl_pattern = r'^(\s*)((?:mutable\s+)?(?:unsigned\s+)?(?:int|long|short|char|float|double|bool|size_t)\s+)(\w+)\s*;'
            
            def add_member_init(match):
                indent = match.group(1)
                type_str = match.group(2)
                name = match.group(3)
                
                if 'bool' in type_str:
                    init_val = 'false'
                else:
                    init_val = '0'
                    
                return f"{indent}{type_str}{name} = {init_val};"
            
            # Apply to class/struct bodies only
            lines = content.split('\n')
            in_class = False
            new_lines = []
            
            for line in lines:
                if re.match(r'\s*(class|struct)\s+\w+', line):
                    in_class = True
                elif in_class and line.strip() == '};':
                    in_class = False
                    
                if in_class and not any(skip in line for skip in ['static', 'virtual', 'explicit']):
                    line = re.sub(member_decl_pattern, add_member_init, line)
                    
                new_lines.append(line)
                
            content = '\n'.join(new_lines)
            
            if content != original_content:
                with open(filepath, 'w') as f:
                    f.write(content)
                self.files_modified += 1
                return True
                
        except Exception as e:
            print(f"Error processing {filepath}: {e}")
            
        return False
    
    def process_directory(self, directory: str):
        """Process all C++ files in directory"""
        for root, dirs, files in os.walk(directory):
            # Skip hidden and build directories
            dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['build', 'CMakeFiles']]
            
            for file in files:
                if file.endswith(('.h', '.hpp', '.cpp', '.cc')):
                    filepath = os.path.join(root, file)
                    if self.fix_file(filepath):
                        print(f"Fixed: {filepath}")

def main():
    fixer = UninitMemberFixer()
    
    os.chdir('/Users/jerry/lpzrobot_mac')
    
    components = ['selforg', 'ga_tools', 'opende', 'ode_robots']
    
    for component in components:
        print(f"\nProcessing {component}...")
        fixer.process_directory(component)
    
    print(f"\nTotal files modified: {fixer.files_modified}")
    print(f"Total members initialized: {fixer.members_initialized}")

if __name__ == '__main__':
    main()