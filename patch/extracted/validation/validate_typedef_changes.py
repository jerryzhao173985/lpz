#!/usr/bin/env python3
"""
Validate typedef to using conversions to ensure they are correct.
"""

import re
import sys

# Common typedef patterns and their correct conversions
typedef_patterns = {
    # Simple type alias: typedef type name;
    r'typedef\s+(.+?)\s+([A-Za-z_]\w*);': r'using \2 = \1;',
    
    # Function pointer: typedef return_type (*name)(params);
    r'typedef\s+(.+?)\s*\(\*([A-Za-z_]\w*)\)\s*\((.*?)\);': r'using \2 = \1 (*)(\3);',
    
    # Array typedef: typedef type name[size];
    r'typedef\s+(.+?)\s+([A-Za-z_]\w*)\[(\d+)\];': r'using \2 = \1[\3];',
}

def validate_typedef_conversion(old_line, new_line):
    """Validate that a typedef was correctly converted to using."""
    old_line = old_line.strip()
    new_line = new_line.strip()
    
    # Skip if not a typedef line
    if not old_line.startswith('typedef'):
        return True, "Not a typedef line"
    
    # Check each pattern
    for pattern, replacement in typedef_patterns.items():
        match = re.match(pattern, old_line)
        if match:
            expected = re.sub(pattern, replacement, old_line)
            if new_line == expected:
                return True, f"Correct conversion: {pattern}"
            else:
                return False, f"Expected: {expected}, Got: {new_line}"
    
    # If no pattern matched, it might be a complex typedef
    return None, "Complex typedef - manual review needed"

def analyze_patch(patch_file):
    """Analyze the patch file and validate conversions."""
    with open(patch_file, 'r') as f:
        lines = f.readlines()
    
    issues = []
    validations = []
    current_file = None
    line_num = 0
    
    i = 0
    while i < len(lines):
        line = lines[i]
        
        if line.startswith('diff --git'):
            current_file = line.split()[2]
        elif line.startswith('@@'):
            # Extract line number from hunk header
            match = re.search(r'-(\d+)', line)
            if match:
                line_num = int(match.group(1))
        elif line.startswith('-') and 'typedef' in line:
            # Found a typedef removal, look for corresponding using addition
            typedef_line = line[1:].strip()
            
            # Skip if this is in a comment or documentation
            if any(x in typedef_line for x in ['*', '//', '#', 'NOTE:', '```', '- ', '### ']):
                continue
            
            # Skip if not a real typedef declaration (must end with ; or have struct/enum)
            if not (typedef_line.endswith(';') or 'struct' in typedef_line or 'enum' in typedef_line):
                continue
            
            # Look ahead for the using declaration
            found_using = False
            for j in range(i+1, min(i+10, len(lines))):
                if lines[j].startswith('+') and 'using' in lines[j]:
                    using_line = lines[j][1:].strip()
                    
                    # Validate the conversion
                    is_valid, message = validate_typedef_conversion(typedef_line, using_line)
                    
                    if is_valid is True:
                        validations.append(f"✓ {current_file}:{line_num} - {message}")
                    elif is_valid is False:
                        issues.append(f"✗ {current_file}:{line_num} - {message}")
                        issues.append(f"  Old: {typedef_line}")
                        issues.append(f"  New: {using_line}")
                    else:
                        validations.append(f"? {current_file}:{line_num} - {message}")
                        validations.append(f"  Old: {typedef_line}")
                        validations.append(f"  New: {using_line}")
                    
                    found_using = True
                    break
            
            if not found_using:
                issues.append(f"✗ {current_file}:{line_num} - No corresponding using declaration found")
                issues.append(f"  Old: {typedef_line}")
        
        if line.startswith('-') or line.startswith(' '):
            line_num += 1
        
        i += 1
    
    return validations, issues

if __name__ == "__main__":
    patch_file = 'patch/extracted/patch_01_typedef_to_using.patch'
    
    print("Validating typedef to using conversions...")
    validations, issues = analyze_patch(patch_file)
    
    # Write validation report
    with open('patch/extracted/validation/typedef_validation_report.txt', 'w') as f:
        f.write("TYPEDEF TO USING CONVERSION VALIDATION REPORT\n")
        f.write("=" * 50 + "\n\n")
        
        f.write(f"Total validations: {len(validations)}\n")
        f.write(f"Issues found: {len(issues)}\n\n")
        
        if issues:
            f.write("ISSUES:\n")
            f.write("-" * 30 + "\n")
            for issue in issues:
                f.write(issue + "\n")
            f.write("\n")
        
        f.write("VALIDATIONS:\n")
        f.write("-" * 30 + "\n")
        for validation in validations[:20]:  # Show first 20
            f.write(validation + "\n")
        
        if len(validations) > 20:
            f.write(f"\n... and {len(validations) - 20} more validations\n")
    
    print(f"Validation complete:")
    print(f"  - Valid conversions: {len(validations)}")
    print(f"  - Issues found: {len(issues)}")
    print(f"  - Report written to: patch/extracted/validation/typedef_validation_report.txt")
    
    if issues:
        print("\nISSUES FOUND - Please review the report!")
        sys.exit(1)