#!/usr/bin/env python3
import os
import re
import random

def find_traditional_loops(directory, sample_size=10):
    """Find traditional for loops that could be modernized"""
    examples = []
    pattern = re.compile(r'for\s*\(\s*int\s+(\w+)\s*=\s*0\s*;\s*\1\s*<\s*(\w+)\.size\(\)\s*;\s*\+\+\1\s*\)')
    
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.cpp', '.h')) and 'patch' not in root:
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r') as f:
                        content = f.read()
                        for match in pattern.finditer(content):
                            line_num = content[:match.start()].count('\n') + 1
                            examples.append({
                                'file': filepath.replace(directory + '/', ''),
                                'line': line_num,
                                'code': match.group(0),
                                'var': match.group(1),
                                'container': match.group(2)
                            })
                except:
                    pass
    
    return random.sample(examples, min(sample_size, len(examples)))

def find_typedef_examples(directory, sample_size=5):
    """Find typedef declarations that could be 'using'"""
    examples = []
    pattern = re.compile(r'typedef\s+([^;]+)\s+(\w+);')
    
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.h', '.hpp')) and 'patch' not in root:
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r') as f:
                        content = f.read()
                        for match in pattern.finditer(content):
                            line_num = content[:match.start()].count('\n') + 1
                            examples.append({
                                'file': filepath.replace(directory + '/', ''),
                                'line': line_num,
                                'old': match.group(0),
                                'type': match.group(1).strip(),
                                'name': match.group(2)
                            })
                except:
                    pass
    
    return random.sample(examples, min(sample_size, len(examples)))

def find_auto_opportunities(directory, sample_size=5):
    """Find iterator declarations that could use auto"""
    examples = []
    pattern = re.compile(r'(std::\w+<[^>]+>::(?:const_)?iterator)\s+(\w+)\s*=')
    
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.cpp', '.h')) and 'patch' not in root:
                filepath = os.path.join(root, file)
                try:
                    with open(filepath, 'r') as f:
                        content = f.read()
                        for match in pattern.finditer(content):
                            line_num = content[:match.start()].count('\n') + 1
                            examples.append({
                                'file': filepath.replace(directory + '/', ''),
                                'line': line_num,
                                'type': match.group(1),
                                'var': match.group(2)
                            })
                except:
                    pass
    
    return random.sample(examples, min(sample_size, len(examples)))

if __name__ == "__main__":
    base_dir = "/Users/jerry/simulator/lpz"
    
    print("# Modernization Examples\n")
    
    print("## Traditional For Loops → Range-Based")
    print("Found in 541 files. Here are some examples:\n")
    for ex in find_traditional_loops(base_dir):
        print(f"**{ex['file']}:{ex['line']}**")
        print(f"```cpp")
        print(f"// Current")
        print(f"{ex['code']}")
        print(f"// Could be")
        print(f"for (auto& item : {ex['container']}) {{")
        print(f"```\n")
    
    print("\n## typedef → using")
    print("Found in 316 files. Here are some examples:\n")
    for ex in find_typedef_examples(base_dir):
        print(f"**{ex['file']}:{ex['line']}**")
        print(f"```cpp")
        print(f"// Current")
        print(f"{ex['old']}")
        print(f"// Could be")
        print(f"using {ex['name']} = {ex['type']};")
        print(f"```\n")
    
    print("\n## Iterator Declarations → auto")
    print("Many opportunities for auto. Here are some examples:\n")
    for ex in find_auto_opportunities(base_dir):
        print(f"**{ex['file']}:{ex['line']}**")
        print(f"```cpp")
        print(f"// Current")
        print(f"{ex['type']} {ex['var']} = ...")
        print(f"// Could be")
        print(f"auto {ex['var']} = ...")
        print(f"```\n")