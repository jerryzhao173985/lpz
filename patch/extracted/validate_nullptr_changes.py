#!/usr/bin/env python3
"""
Validate NULL to nullptr conversions to ensure they are correct.
Based on the pattern established in validate_typedef_changes.py
"""

import re
import sys

# Context where NULL to nullptr conversion is valid
valid_contexts = {
    # Assignments
    r'=\s*NULL': r'= nullptr',
    # Comparisons
    r'==\s*NULL': r'== nullptr',
    r'!=\s*NULL': r'!= nullptr',
    r'NULL\s*==': r'nullptr ==',
    r'NULL\s*!=': r'nullptr !=',
    # Return statements
    r'return\s+NULL': r'return nullptr',
    # Function arguments
    r'\(\s*NULL\s*\)': r'(nullptr)',
    r',\s*NULL\s*,': r', nullptr,',
    r',\s*NULL\s*\)': r', nullptr)',
    # Ternary operator
    r'\?\s*NULL\s*:': r'? nullptr :',
    r':\s*NULL': r': nullptr',
    # Initialization
    r'{\s*NULL\s*}': r'{ nullptr }',
    r'=\s*{\s*NULL': r'= { nullptr',
}

# Contexts where NULL should NOT be converted
invalid_contexts = [
    # Macros
    r'#define.*NULL',
    r'#ifdef.*NULL',
    r'#ifndef.*NULL',
    # Comments
    r'//.*NULL',
    r'/\*.*NULL.*\*/',
    # Strings
    r'".*NULL.*"',
    r"'.*NULL.*'",
    # Documentation
    r'\*\s+.*NULL',
]

def validate_nullptr_conversion(old_line, new_line):
    """Validate that NULL was correctly converted to nullptr."""
    old_line = old_line.strip()
    new_line = new_line.strip()
    
    # Skip if not a NULL line
    if 'NULL' not in old_line:
        return True, "No NULL in line"
    
    # Check if it's in an invalid context
    for pattern in invalid_contexts:
        if re.search(pattern, old_line):
            return True, f"NULL in invalid context for conversion: {pattern}"
    
    # Check for arithmetic operations (nullptr can't be used in arithmetic)
    if any(op in old_line for op in [' + ', ' - ', ' * ', ' / ', ' % ', '<<', '>>']):
        if 'nullptr' in new_line:
            return False, "nullptr cannot be used in arithmetic operations"
    
    # Check for cast to integral type (NULL can be cast to int, nullptr cannot)
    if re.search(r'\(int\)\s*NULL', old_line) or re.search(r'static_cast<int>\s*\(\s*NULL\s*\)', old_line):
        if 'nullptr' in new_line:
            return False, "nullptr cannot be cast to integral types"
    
    # Validate the conversion was done correctly
    expected = old_line
    for pattern, replacement in valid_contexts.items():
        expected = re.sub(pattern, replacement, expected)
    
    # Basic check: all NULL should be replaced with nullptr
    expected = expected.replace('NULL', 'nullptr')
    
    if new_line == expected:
        return True, "Correct conversion"
    
    # Check if the lines are essentially the same (whitespace differences)
    if re.sub(r'\s+', ' ', new_line) == re.sub(r'\s+', ' ', expected):
        return True, "Correct conversion (whitespace normalized)"
    
    return None, "Manual review needed"

def analyze_patch(patch_file):
    """Analyze the patch file and validate conversions."""
    with open(patch_file, 'r') as f:
        lines = f.readlines()
    
    issues = []
    validations = []
    warnings = []
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
        elif line.startswith('-') and 'NULL' in line:
            # Found a NULL removal, look for corresponding nullptr addition
            null_line = line[1:].strip()
            
            # Skip if this is in a comment or documentation
            if any(x in null_line for x in ['//', '/*', '*/', '```', '###', 'NOTE:', '- ']):
                i += 1
                continue
            
            # Look ahead for the nullptr replacement
            found_nullptr = False
            for j in range(i+1, min(i+10, len(lines))):
                if lines[j].startswith('+'):
                    plus_line = lines[j][1:].strip()
                    
                    # Check if this could be the replacement
                    if ('nullptr' in plus_line) or (null_line.replace('NULL', 'nullptr') == plus_line):
                        # Validate the conversion
                        is_valid, message = validate_nullptr_conversion(null_line, plus_line)
                        
                        if is_valid is True:
                            validations.append(f"✓ {current_file}:{line_num} - {message}")
                        elif is_valid is False:
                            issues.append(f"✗ {current_file}:{line_num} - {message}")
                            issues.append(f"  Old: {null_line}")
                            issues.append(f"  New: {plus_line}")
                        else:
                            warnings.append(f"? {current_file}:{line_num} - {message}")
                            warnings.append(f"  Old: {null_line}")
                            warnings.append(f"  New: {plus_line}")
                        
                        found_nullptr = True
                        break
            
            if not found_nullptr and 'NULL' in null_line:
                # Check if NULL was removed without replacement (might be okay in some cases)
                if not any(marker in null_line for marker in ['#define', '#ifdef', 'typedef']):
                    warnings.append(f"? {current_file}:{line_num} - NULL removed without nullptr replacement")
                    warnings.append(f"  Old: {null_line}")
        
        if line.startswith('-') or line.startswith(' '):
            line_num += 1
        
        i += 1
    
    return validations, issues, warnings

if __name__ == "__main__":
    patch_file = 'patch/extracted/patch_02_null_to_nullptr.patch'
    
    print("Validating NULL to nullptr conversions...")
    validations, issues, warnings = analyze_patch(patch_file)
    
    # Write validation report
    with open('patch/extracted/validation/nullptr_validation_report.txt', 'w') as f:
        f.write("NULL TO NULLPTR CONVERSION VALIDATION REPORT\n")
        f.write("=" * 50 + "\n\n")
        
        f.write(f"Total validations: {len(validations)}\n")
        f.write(f"Issues found: {len(issues)}\n")
        f.write(f"Warnings (need review): {len(warnings)}\n\n")
        
        if issues:
            f.write("ISSUES:\n")
            f.write("-" * 30 + "\n")
            for issue in issues:
                f.write(issue + "\n")
            f.write("\n")
        
        if warnings:
            f.write("WARNINGS (Manual Review Needed):\n")
            f.write("-" * 30 + "\n")
            for warning in warnings[:20]:  # Show first 20
                f.write(warning + "\n")
            if len(warnings) > 20:
                f.write(f"\n... and {len(warnings) - 20} more warnings\n")
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
    print(f"  - Warnings (need review): {len(warnings)}")
    print(f"  - Report written to: patch/extracted/validation/nullptr_validation_report.txt")
    
    if issues:
        print("\nCRITICAL ISSUES FOUND - Please review the report!")
        sys.exit(1)