#!/usr/bin/env python3
import os
import re
import subprocess

def fix_explicit_removals():
    """Fix the incorrect removal of 'explicit' from control flow statements"""
    
    # Get list of modified files with explicit removals
    files_to_fix = []
    
    # Run git diff to find files with problematic changes
    diff_output = subprocess.run(['git', 'diff', '--name-only'], 
                                capture_output=True, text=True).stdout
    
    for file in diff_output.strip().split('\n'):
        if file and (file.endswith('.cpp') or file.endswith('.h')):
            files_to_fix.append(file)
    
    # Fix each file
    for filepath in files_to_fix:
        if os.path.exists(filepath):
            with open(filepath, 'r') as f:
                content = f.read()
            
            original = content
            
            # These should never have had 'explicit' removed
            # The user incorrectly removed 'explicit' thinking it was part of the bad sed fixes
            # but these were just normal switch/while statements
            patterns_to_revert = [
                (r'(switch\s*\([^)]+\)\s*{)', r'explicit \1'),
                (r'(while\s*\([^)]+\)\s*{)', r'explicit \1'),
                (r'(for\s*\([^)]+\)\s*{)', r'explicit \1')
            ]
            
            # Actually, these should NOT have explicit - revert to normal
            # User was trying to fix "explicit if" but went too far
            content = re.sub(r'explicit\s+(switch|while|for)\s*\(', r'\1(', content)
            
            if content != original:
                with open(filepath, 'w') as f:
                    f.write(content)
                print(f"Fixed {filepath}")

def fix_destructor_overrides():
    """Fix incorrect override usage on destructors"""
    configurator_files = [
        'configurator/src/qconfigurable/QAbstractConfigurableTileWidget.h',
        'configurator/src/qconfigurable/QBoolConfigurableTileWidget.h', 
        'configurator/src/qconfigurable/QChangeNumberTileColumnsDialog.h',
        'configurator/src/qconfigurable/QConfigurableTileShowHideDialog.h',
        'configurator/src/qconfigurable/QConfigurableWidget.h',
        'configurator/src/qconfigurable/QIntConfigurableTileWidget.h',
        'configurator/src/qconfigurable/QValConfigurableTileWidget.h'
    ]
    
    for filepath in configurator_files:
        if os.path.exists(filepath):
            with open(filepath, 'r') as f:
                content = f.read()
            
            original = content
            
            # Virtual destructors should not have override
            content = re.sub(r'virtual\s+~(\w+)\(\)\s*override\s*;', r'virtual ~\1();', content)
            
            if content != original:
                with open(filepath, 'w') as f:
                    f.write(content)
                print(f"Fixed destructor override in {filepath}")

def fix_virtual_overrides():
    """Fix virtual functions that should have override"""
    
    # These files have virtual functions that DO override base class methods
    override_fixes = {
        'configurator/src/qconfigurable/QAbstractConfigurableTileWidget.h': {
            # These are QWidget overrides
            'setVisible': True,
            'enterEvent': False,  # This one needs to stay without override  
            'leaveEvent': True,
            'mouseMoveEvent': True,
            'mousePressEvent': True
        },
        'configurator/src/qconfigurable/QBoolConfigurableTileWidget.h': {
            'setName': True,
            'toDummy': True,
            'reloadConfigurableData': True,
            'sl_resetToOriginalValues': True
        },
        'configurator/src/qconfigurable/QIntConfigurableTileWidget.h': {
            'setName': True,
            'toDummy': True,
            'reloadConfigurableData': True,
            'sl_resize': True,
            'sl_resetToOriginalValues': True
        },
        'configurator/src/qconfigurable/QValConfigurableTileWidget.h': {
            'setName': True,
            'toDummy': True,
            'reloadConfigurableData': True,
            'sl_resize': True,
            'sl_resetToOriginalValues': True
        },
        'configurator/src/qconfigurable/QConfigurableWidget.h': {
            'doOnCallBack': True,
            'enterEvent': True,
            'leaveEvent': True,
            'mousePressEvent': True,
            'mouseDoubleClickEvent': True,
            'dragEnterEvent': True,
            'dragMoveEvent': True,
            'dropEvent': True,
            'dragLeaveEvent': True
        },
        'ecbrobots/ecbagent.h': {
            'step': True
        }
    }
    
    for filepath, methods in override_fixes.items():
        if os.path.exists(filepath):
            with open(filepath, 'r') as f:
                content = f.read()
            
            original = content
            
            for method, should_have_override in methods.items():
                if should_have_override:
                    # Add override if missing
                    pattern = rf'(virtual\s+[\w\s\*:<>&]+\s+{method}\s*\([^)]*\)(?:\s*const)?)\s*;'
                    replacement = r'\1 override;'
                    content = re.sub(pattern, replacement, content)
                
            if content != original:
                with open(filepath, 'w') as f:
                    f.write(content)
                print(f"Fixed virtual overrides in {filepath}")

def main():
    print("Fixing user changes...")
    
    # First fix the explicit removals (these were wrong)
    fix_explicit_removals()
    
    # Fix destructor overrides
    fix_destructor_overrides()
    
    # Fix virtual function overrides
    fix_virtual_overrides()
    
    print("\nDone! Please review the changes with 'git diff'")

if __name__ == "__main__":
    main()