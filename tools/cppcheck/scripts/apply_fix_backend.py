#!/usr/bin/env python3
"""
Apply Fix Backend for Cppcheck Dashboard
Safely applies fixes to source files with backup and rollback capabilities
"""

import json
import os
import sys
import shutil
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Tuple
import difflib
import tempfile

class FixApplicator:
    def __init__(self, project_root: Path, dry_run: bool = True):
        self.project_root = project_root
        self.dry_run = dry_run
        self.backup_dir = project_root / '.cppcheck_backups' / datetime.now().strftime('%Y%m%d_%H%M%S')
        self.applied_fixes = self._load_applied_fixes()
        
    def _load_applied_fixes(self) -> Dict:
        """Load record of previously applied fixes"""
        fixes_file = self.project_root / '.cppcheck_fixes.json'
        if fixes_file.exists():
            with open(fixes_file) as f:
                return json.load(f)
        return {'fixes': [], 'by_file': {}}
        
    def _save_applied_fixes(self):
        """Save record of applied fixes"""
        fixes_file = self.project_root / '.cppcheck_fixes.json'
        with open(fixes_file, 'w') as f:
            json.dump(self.applied_fixes, f, indent=2)
            
    def _backup_file(self, filepath: Path) -> Path:
        """Create backup of file before modification"""
        relative_path = filepath.relative_to(self.project_root)
        backup_path = self.backup_dir / relative_path
        backup_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(filepath, backup_path)
        return backup_path
        
    def apply_fix(self, fix_data: Dict) -> Dict:
        """Apply a single fix to a file"""
        result = {
            'success': False,
            'message': '',
            'backup_path': None,
            'diff_applied': None
        }
        
        try:
            # Extract fix information
            filepath = fix_data.get('filepath')
            if not filepath:
                result['message'] = 'No filepath provided'
                return result
                
            file_path = Path(filepath)
            if not file_path.is_absolute():
                file_path = self.project_root / file_path
                
            if not file_path.exists():
                result['message'] = f'File not found: {filepath}'
                return result
                
            # Check if fix was already applied
            fix_id = f"{filepath}:{fix_data.get('line', 0)}:{fix_data.get('issue_id', 'unknown')}"
            if fix_id in self.applied_fixes.get('fixes', []):
                result['message'] = 'Fix already applied'
                result['success'] = True
                return result
                
            # Get the diff
            diff_text = fix_data.get('diff', '')
            if not diff_text:
                result['message'] = 'No diff provided'
                return result
                
            # Parse the unified diff to extract changes
            original_lines = []
            fixed_lines = []
            target_line = int(fix_data.get('line', 0))
            
            # Read current file content
            with open(file_path, 'r', encoding='utf-8') as f:
                current_lines = f.readlines()
                
            # Apply the fix based on fix data
            if 'fixed_lines' in fix_data:
                # We have the specific fixed lines
                fixed_content = self._apply_fixed_lines(
                    current_lines, 
                    fix_data['fixed_lines'],
                    target_line
                )
            else:
                # Apply diff patch
                fixed_content = self._apply_diff_patch(current_lines, diff_text)
                
            if not fixed_content:
                result['message'] = 'Failed to apply fix'
                return result
                
            # Create backup
            if not self.dry_run:
                backup_path = self._backup_file(file_path)
                result['backup_path'] = str(backup_path)
                
            # Write fixed content
            if self.dry_run:
                # In dry run, create a temp file to show the result
                with tempfile.NamedTemporaryFile(mode='w', suffix='.cpp', delete=False) as tmp:
                    tmp.write(fixed_content)
                    result['temp_file'] = tmp.name
                result['message'] = 'Dry run - no files modified'
            else:
                with open(file_path, 'w', encoding='utf-8') as f:
                    f.write(fixed_content)
                    
                # Record the fix
                self.applied_fixes['fixes'].append(fix_id)
                if filepath not in self.applied_fixes['by_file']:
                    self.applied_fixes['by_file'][filepath] = []
                self.applied_fixes['by_file'][filepath].append({
                    'line': target_line,
                    'issue_id': fix_data.get('issue_id'),
                    'timestamp': datetime.now().isoformat(),
                    'description': fix_data.get('description', 'Unknown fix')
                })
                self._save_applied_fixes()
                
                result['message'] = 'Fix applied successfully'
                
            # Generate actual diff applied
            result['diff_applied'] = '\n'.join(difflib.unified_diff(
                current_lines,
                fixed_content.splitlines(keepends=True),
                fromfile=f'a/{filepath}',
                tofile=f'b/{filepath}',
                lineterm=''
            ))
            
            result['success'] = True
            
        except Exception as e:
            result['message'] = f'Error applying fix: {str(e)}'
            
        return result
        
    def _apply_fixed_lines(self, current_lines: List[str], fixed_lines: List[Dict], 
                          target_line: int) -> Optional[str]:
        """Apply fixed lines to current content"""
        # Build a mapping of what lines to replace
        replacements = {}
        for line_info in fixed_lines:
            if line_info.get('modified'):
                line_num = line_info['number'] - 1  # Convert to 0-based
                if 0 <= line_num < len(current_lines):
                    replacements[line_num] = line_info['content'] + '\n'
                    
        # Apply replacements
        result_lines = []
        for i, line in enumerate(current_lines):
            if i in replacements:
                result_lines.append(replacements[i])
            else:
                result_lines.append(line)
                
        return ''.join(result_lines)
        
    def _apply_diff_patch(self, current_lines: List[str], diff_text: str) -> Optional[str]:
        """Apply a unified diff patch to current content"""
        try:
            # Use Python's difflib to parse and apply the patch
            # This is a simplified implementation - for production use patch command
            import re
            
            result_lines = current_lines.copy()
            
            # Parse the diff to find changes
            diff_lines = diff_text.split('\n')
            i = 0
            while i < len(diff_lines):
                line = diff_lines[i]
                
                # Look for hunk headers
                if line.startswith('@@'):
                    # Parse hunk header: @@ -start,count +start,count @@
                    match = re.match(r'@@ -(\d+),?\d* \+(\d+),?\d* @@', line)
                    if match:
                        old_start = int(match.group(1)) - 1  # Convert to 0-based
                        new_start = int(match.group(2)) - 1
                        
                        # Process the hunk
                        j = i + 1
                        while j < len(diff_lines) and not diff_lines[j].startswith('@@'):
                            hunk_line = diff_lines[j]
                            if hunk_line.startswith('-'):
                                # Remove line
                                if old_start < len(result_lines):
                                    result_lines[old_start] = None  # Mark for removal
                            elif hunk_line.startswith('+'):
                                # Add line
                                content = hunk_line[1:] + '\n'
                                if old_start < len(result_lines):
                                    if result_lines[old_start] is None:
                                        result_lines[old_start] = content
                                    else:
                                        result_lines.insert(old_start, content)
                                else:
                                    result_lines.append(content)
                                old_start += 1
                            elif hunk_line.startswith(' '):
                                # Context line
                                old_start += 1
                            j += 1
                        i = j - 1
                i += 1
                
            # Remove None entries (deleted lines)
            result_lines = [line for line in result_lines if line is not None]
            
            return ''.join(result_lines)
            
        except Exception as e:
            print(f"Error applying patch: {e}")
            return None
            
    def rollback_fix(self, filepath: str, backup_path: str) -> bool:
        """Rollback a previously applied fix"""
        try:
            if Path(backup_path).exists():
                shutil.copy2(backup_path, filepath)
                
                # Update applied fixes record
                if filepath in self.applied_fixes['by_file']:
                    # Remove the last fix for this file
                    self.applied_fixes['by_file'][filepath].pop()
                    if not self.applied_fixes['by_file'][filepath]:
                        del self.applied_fixes['by_file'][filepath]
                        
                self._save_applied_fixes()
                return True
        except Exception as e:
            print(f"Error rolling back fix: {e}")
            
        return False
        
    def get_fix_history(self, filepath: Optional[str] = None) -> List[Dict]:
        """Get history of applied fixes"""
        if filepath:
            return self.applied_fixes.get('by_file', {}).get(filepath, [])
        else:
            # Return all fixes
            all_fixes = []
            for file, fixes in self.applied_fixes.get('by_file', {}).items():
                for fix in fixes:
                    all_fixes.append({
                        'file': file,
                        **fix
                    })
            return sorted(all_fixes, key=lambda x: x.get('timestamp', ''), reverse=True)


def main():
    """CLI interface for applying fixes"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Apply Cppcheck fixes')
    parser.add_argument('action', choices=['apply', 'rollback', 'history'],
                       help='Action to perform')
    parser.add_argument('--fix-data', type=str,
                       help='JSON string or file with fix data')
    parser.add_argument('--file', type=str,
                       help='File to apply fix to')
    parser.add_argument('--line', type=int,
                       help='Line number of fix')
    parser.add_argument('--dry-run', action='store_true',
                       help='Show what would be done without modifying files')
    parser.add_argument('--project-root', type=str, default='.',
                       help='Project root directory')
    
    args = parser.parse_args()
    
    project_root = Path(args.project_root).resolve()
    applicator = FixApplicator(project_root, dry_run=args.dry_run)
    
    if args.action == 'apply':
        if args.fix_data:
            # Load fix data
            if args.fix_data.startswith('{'):
                fix_data = json.loads(args.fix_data)
            else:
                with open(args.fix_data) as f:
                    fix_data = json.load(f)
                    
            result = applicator.apply_fix(fix_data)
            print(json.dumps(result, indent=2))
            
    elif args.action == 'history':
        history = applicator.get_fix_history(args.file)
        print(json.dumps(history, indent=2))
        
    elif args.action == 'rollback':
        # TODO: Implement rollback
        print("Rollback not yet implemented")


if __name__ == '__main__':
    main()