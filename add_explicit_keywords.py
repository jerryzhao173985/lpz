#!/usr/bin/env python3
import re
import os

def add_explicit_to_constructor(filepath, class_name, line_number):
    """Add explicit keyword to a constructor at the given line."""
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Adjust for 0-based indexing
    line_idx = line_number - 1
    
    if line_idx < 0 or line_idx >= len(lines):
        print(f"  Error: Line {line_number} out of range in {filepath}")
        return False
    
    line = lines[line_idx]
    
    # Check if explicit already exists
    if 'explicit' in line:
        print(f"  Already has explicit: {line.strip()}")
        return False
    
    # Add explicit before the constructor
    # Pattern to match constructor declaration
    pattern = r'^(\s*)([\w:]+\s*\()'
    match = re.match(pattern, line)
    
    if match:
        indent = match.group(1)
        rest = match.group(2)
        lines[line_idx] = f"{indent}explicit {rest}{line[len(match.group(0)):]}"
        
        with open(filepath, 'w') as f:
            f.writelines(lines)
        
        print(f"  Added explicit at line {line_number}")
        return True
    else:
        print(f"  Could not match constructor pattern at line {line_number}: {line.strip()}")
        return False

def main():
    # List of files and line numbers from cppcheck output
    constructors_to_fix = [
        ("selforg/wiredcontroller.h", 63),
        ("selforg/wiredcontroller.h", 67),
        ("selforg/agent.h", 57),
        ("selforg/agent.h", 61),
        ("selforg/controller/measureadapter.h", 49),
        ("selforg/controller/mutualinformationcontroller.h", 53),
        ("selforg/controller/oneactivemultipassivecontroller.h", 39),
        ("selforg/controller/som.h", 41),
        ("selforg/controller/switchcontroller.h", 36),
        ("selforg/controller/use_java_controller.h", 75),
        ("selforg/statistictools/dataanalysation/templatevalueanalysation.h", 169),
        ("selforg/statistictools/measure/abstractmeasure.h", 43),
        ("selforg/statistictools/statistictools.h", 48),
        ("selforg/utils/inspectable.h", 107),
        ("selforg/utils/noisegenerator.h", 165),
        ("selforg/utils/configurablelist.h", 41),
        ("selforg/utils/inspectableproxy.h", 31),
        ("selforg/utils/inspectableproxy.h", 33),
        ("selforg/wirings/abstractwiring.h", 57),
        ("selforg/wirings/copywiring.h", 38),
    ]
    
    fixed_count = 0
    for filepath, line_number in constructors_to_fix:
        if os.path.exists(filepath):
            print(f"\nProcessing {filepath} line {line_number}:")
            if add_explicit_to_constructor(filepath, "", line_number):
                fixed_count += 1
        else:
            print(f"\nFile not found: {filepath}")
    
    print(f"\n\nTotal constructors fixed: {fixed_count}")

if __name__ == "__main__":
    main()