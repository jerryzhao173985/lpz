#!/usr/bin/env python3
"""
LPZRobots Cppcheck Analysis Engine
Advanced static analysis with caching, incremental checks, and modular profiles
"""

import argparse
import json
import os
import sys
import subprocess
import hashlib
import time
import multiprocessing
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Set, Tuple, Optional
import xml.etree.ElementTree as ET

# Constants
SCRIPT_DIR = Path(__file__).parent
TOOLS_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TOOLS_DIR.parent.parent  # Fixed: go up one more level
CACHE_DIR = TOOLS_DIR / "cache"
REPORTS_DIR = TOOLS_DIR / "reports"
PROFILES_DIR = TOOLS_DIR / "profiles"
CONFIGS_DIR = TOOLS_DIR / "configs"

# Colors for terminal output
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    MAGENTA = '\033[0;35m'
    CYAN = '\033[0;36m'
    NC = '\033[0m'  # No Color

class CppcheckAnalyzer:
    def __init__(self, profile: str = "quick_check", verbose: bool = False):
        self.profile_name = profile
        self.verbose = verbose
        self.profile = self._load_profile(profile)
        self.cache = self._init_cache()
        self.stats = {
            'files_analyzed': 0,
            'files_cached': 0,
            'errors': 0,
            'warnings': 0,
            'style': 0,
            'performance': 0,
            'start_time': time.time()
        }
        
    def _load_profile(self, profile_name: str) -> Dict:
        """Load analysis profile configuration"""
        profile_path = PROFILES_DIR / f"{profile_name}.json"
        if not profile_path.exists():
            print(f"{Colors.RED}Profile '{profile_name}' not found{Colors.NC}")
            sys.exit(1)
            
        with open(profile_path) as f:
            return json.load(f)
            
    def _init_cache(self) -> Dict:
        """Initialize caching system"""
        cache_file = CACHE_DIR / f"{self.profile_name}_cache.json"
        if cache_file.exists():
            with open(cache_file) as f:
                return json.load(f)
        return {}
        
    def _save_cache(self):
        """Save cache to disk"""
        cache_file = CACHE_DIR / f"{self.profile_name}_cache.json"
        with open(cache_file, 'w') as f:
            json.dump(self.cache, f, indent=2)
            
    def _get_file_hash(self, filepath: Path) -> str:
        """Calculate hash of file content for cache invalidation"""
        hasher = hashlib.md5()
        with open(filepath, 'rb') as f:
            hasher.update(f.read())
        return hasher.hexdigest()
        
    def _get_changed_files(self) -> Set[Path]:
        """Get list of changed files since last analysis"""
        changed = set()
        
        # Use git to find changed files if available
        try:
            result = subprocess.run(
                ['git', 'diff', '--name-only', '--diff-filter=ACMRT'],
                capture_output=True, text=True, cwd=PROJECT_ROOT
            )
            if result.returncode == 0:
                for line in result.stdout.strip().split('\n'):
                    if line and any(line.endswith(ext) for ext in ['.cpp', '.h', '.hpp', '.cc', '.cxx']):
                        changed.add(PROJECT_ROOT / line)
        except:
            pass
            
        # Also check file modification times
        for component in ['selforg', 'ode_robots', 'ga_tools', 'guilogger', 'matrixviz']:
            component_path = PROJECT_ROOT / component
            if component_path.exists():
                for filepath in component_path.rglob('*.[ch]pp'):
                    file_key = str(filepath.relative_to(PROJECT_ROOT))
                    file_hash = self._get_file_hash(filepath)
                    
                    if file_key not in self.cache or self.cache[file_key].get('hash') != file_hash:
                        changed.add(filepath)
                        
        return changed
        
    def _build_cppcheck_command(self, files: Optional[List[Path]] = None) -> List[str]:
        """Build cppcheck command based on profile"""
        cmd = ['cppcheck']
        
        # Use compile_commands.json if available (preferred)
        compile_db = PROJECT_ROOT / 'build' / 'cppcheck' / 'compile_commands.json'
        # Temporarily disable compile_commands.json as it may not include all files
        use_compile_db = False  # compile_db.exists()
        
        if use_compile_db:
            cmd.extend(['--project', str(compile_db)])
            if self.verbose:
                print(f"{Colors.YELLOW}Using compile_commands.json{Colors.NC}")
        else:
            # Only use base.xml if no compile_commands.json
            base_config = CONFIGS_DIR / 'base.xml'
            if base_config.exists() and False:  # Disabled for now, causes issues
                cmd.extend(['--project', str(base_config)])
            
        # Enable checks based on profile
        if 'enable' in self.profile['checks']:
            cmd.append(f"--enable={','.join(self.profile['checks']['enable'])}")
            
        # Standard and platform
        cmd.extend(['--std=c++17', '--platform=unix64'])
        
        # Settings from profile
        settings = self.profile.get('settings', {})
        if settings.get('bug-hunting'):
            cmd.append('--bug-hunting')
        if 'check-level' in settings:
            cmd.append(f"--check-level={settings['check-level']}")
        if 'max-ctu-depth' in settings:
            cmd.append(f"--max-ctu-depth={settings['max-ctu-depth']}")
            
        # Suppressions
        for supp in self.profile.get('suppressions', []):
            cmd.append(f"--suppress={supp}")
            
        # Output format
        output = self.profile.get('output', {})
        if 'template' in output:
            cmd.append(f"--template={output['template']}")
            
        # Additional options
        cmd.extend([
            '--force',
            '--inline-suppr',
            f'-j{multiprocessing.cpu_count()}',
            '--report-progress' if self.verbose else '--quiet'
        ])
        
        # Add include paths if not using compile_commands.json
        if not use_compile_db:
            # Add common include directories
            include_dirs = [
                PROJECT_ROOT / 'include',
                PROJECT_ROOT / 'selforg' / 'include',
                PROJECT_ROOT / 'ode_robots' / 'include',
                PROJECT_ROOT / 'ga_tools' / 'include',
                PROJECT_ROOT / 'selforg',
                PROJECT_ROOT / 'ode_robots',
                PROJECT_ROOT / 'opende',
            ]
            for inc_dir in include_dirs:
                if inc_dir.exists():
                    cmd.extend(['-I', str(inc_dir)])
        
        # Files to analyze
        if files:
            cmd.extend([str(f) for f in files])
        else:
            # Analyze components from profile
            for component in ['selforg', 'ode_robots', 'ga_tools']:
                component_path = PROJECT_ROOT / component
                if component_path.exists():
                    cmd.append(str(component_path))
                    if self.verbose:
                        print(f"{Colors.CYAN}Adding component: {component_path}{Colors.NC}")
                else:
                    if self.verbose:
                        print(f"{Colors.RED}Component not found: {component_path}{Colors.NC}")
                    
        return cmd
        
    def _parse_output(self, output: str) -> List[Dict]:
        """Parse cppcheck output into structured format"""
        issues = []
        for line in output.strip().split('\n'):
            if not line or 'Checking' in line or 'done checking' in line or 'files checked' in line:
                continue
                
            # Skip progress messages
            if '%' in line and 'files checked' in line:
                continue
                
            # Parse based on template format: {file}:{line}: {severity}: {message}
            if ':' in line:
                parts = line.split(':', 3)
                if len(parts) >= 3:
                    # For format: file:line: severity: message
                    # parts[0] = file, parts[1] = line, parts[2] = " severity", parts[3] = " message"
                    if len(parts) == 4:
                        issue = {
                            'file': parts[0].strip(),
                            'line': parts[1].strip(),
                            'severity': parts[2].strip(),
                            'message': parts[3].strip()
                        }
                    else:
                        # Fallback for other formats
                        issue = {
                            'file': parts[0].strip(),
                            'line': parts[1].strip(),
                            'message': parts[2].strip(),
                            'severity': 'unknown'
                        }
                        
                        # Extract severity and id from template format
                        # Format: {file}:{line}: [{severity}] ({id}) {message}
                        if ': [' in line:
                            # Split by ': [' to get the severity part
                            severity_part = line.split(': [', 1)[1]
                            if ']' in severity_part:
                                issue['severity'] = severity_part.split(']')[0]
                                # Extract id and message
                                remaining = severity_part.split('] ', 1)[1] if '] ' in severity_part else ''
                                if remaining.startswith('(') and ')' in remaining:
                                    issue['id'] = remaining[1:remaining.index(')')]
                                    issue['message'] = remaining[remaining.index(')')+1:].strip()
                                else:
                                    issue['message'] = remaining
                        else:
                            # Try to detect severity from message content
                            msg_lower = issue['message'].lower()
                            if 'error' in msg_lower or 'undefined' in msg_lower:
                                issue['severity'] = 'error'
                            elif 'warning' in msg_lower:
                                issue['severity'] = 'warning'
                            elif 'performance' in msg_lower:
                                issue['severity'] = 'performance'
                            elif 'style' in msg_lower:
                                issue['severity'] = 'style'
                    
                    # Extract ID if present in message
                    if 'message' in issue and issue['message']:
                        # Look for [id] pattern at start of message
                        import re
                        id_match = re.match(r'^\[([^\]]+)\]\s*(.*)$', issue['message'])
                        if id_match:
                            issue['id'] = id_match.group(1)
                            issue['message'] = id_match.group(2)
                            
                    issues.append(issue)
                    
        return issues
        
    def _update_stats(self, issues: List[Dict]):
        """Update statistics from issues"""
        for issue in issues:
            severity = issue.get('severity', 'unknown')
            if severity in self.stats:
                self.stats[severity] += 1
                
    def analyze(self, incremental: bool = True, files: Optional[List[str]] = None) -> Dict:
        """Run analysis with optional incremental mode"""
        print(f"{Colors.BLUE}Starting {self.profile_name} analysis...{Colors.NC}")
        
        # Determine files to analyze
        if incremental and not files:
            changed_files = self._get_changed_files()
            if not changed_files:
                print(f"{Colors.GREEN}No changed files to analyze{Colors.NC}")
                return {'issues': [], 'stats': self.stats}
            files_to_analyze = list(changed_files)
            print(f"{Colors.YELLOW}Analyzing {len(files_to_analyze)} changed files{Colors.NC}")
        else:
            files_to_analyze = [Path(f) for f in files] if files else None
            
        # Build and run command
        cmd = self._build_cppcheck_command(files_to_analyze)
        
        if self.verbose:
            print(f"{Colors.CYAN}Command: {' '.join(cmd)}{Colors.NC}")
            
        # Run cppcheck
        start_time = time.time()
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=PROJECT_ROOT)
        analysis_time = time.time() - start_time
        
        # Parse output
        output = result.stderr + result.stdout
        issues = self._parse_output(output)
        
        # Update cache for analyzed files
        if incremental and files_to_analyze:
            for filepath in files_to_analyze:
                file_key = str(filepath.relative_to(PROJECT_ROOT))
                self.cache[file_key] = {
                    'hash': self._get_file_hash(filepath),
                    'timestamp': datetime.now().isoformat(),
                    'issues': [i for i in issues if i['file'] == file_key]
                }
            self._save_cache()
            
        # Update statistics
        self._update_stats(issues)
        self.stats['analysis_time'] = analysis_time
        self.stats['files_analyzed'] = len(files_to_analyze) if files_to_analyze else 0
        
        return {
            'issues': issues,
            'stats': self.stats,
            'profile': self.profile_name
        }
        
    def generate_report(self, results: Dict, format: str = 'text') -> Path:
        """Generate analysis report in specified format"""
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        report_dir = REPORTS_DIR / self.profile_name / timestamp
        report_dir.mkdir(parents=True, exist_ok=True)
        
        # Always save JSON for metrics tracking
        json_path = report_dir / 'report.json'
        with open(json_path, 'w') as f:
            json.dump(results, f, indent=2)
        
        if format == 'text':
            report_path = report_dir / 'report.txt'
            with open(report_path, 'w') as f:
                f.write(f"LPZRobots Cppcheck Analysis Report\n")
                f.write(f"Profile: {self.profile_name}\n")
                f.write(f"Date: {datetime.now()}\n")
                f.write(f"{'='*60}\n\n")
                
                f.write("Statistics:\n")
                for key, value in results['stats'].items():
                    f.write(f"  {key}: {value}\n")
                f.write(f"\n{'='*60}\n\n")
                
                f.write("Issues:\n")
                for issue in sorted(results['issues'], key=lambda x: (x.get('severity', 'unknown'), x.get('file', ''))):
                    f.write(f"{issue.get('file', 'unknown')}:{issue.get('line', '0')}: [{issue.get('severity', 'unknown')}] {issue.get('message', '')}\n")
                    
        elif format == 'json':
            report_path = json_path
                
        elif format == 'html':
            # Generate the enhanced dashboard with interactive features
            try:
                sys.path.insert(0, str(SCRIPT_DIR))
                from generate_enhanced_dashboard import EnhancedDashboardGenerator
                
                report_path = report_dir / 'report.html'
                print(f"{Colors.BLUE}Generating enhanced dashboard with code preview...{Colors.NC}")
                generator = EnhancedDashboardGenerator(results)
                generator.generate_enhanced_dashboard(report_path)
                print(f"{Colors.GREEN}Enhanced dashboard generated: {report_path}{Colors.NC}")
            except Exception as e:
                # Try ultimate dashboard as fallback
                try:
                    from generate_ultimate_report import UltimateDashboardGenerator
                    generator = UltimateDashboardGenerator(results)
                    generator.generate_ultimate_dashboard(report_path)
                    print(f"{Colors.GREEN}Ultimate dashboard generated: {report_path}{Colors.NC}")
                except:
                    # Final fallback to basic HTML
                    if self.verbose:
                        print(f"{Colors.YELLOW}Could not generate enhanced dashboard: {e}{Colors.NC}")
                        print(f"{Colors.YELLOW}Falling back to basic HTML report{Colors.NC}")
                    report_path = report_dir / 'report.html'
                    self._generate_html_report(results, report_path)
            
        print(f"{Colors.GREEN}Report generated: {report_path}{Colors.NC}")
        return report_path
        
    def _generate_html_report(self, results: Dict, output_path: Path):
        """Generate enhanced HTML report with visualizations"""
        # Count issues by severity and component
        issue_counts = {}
        component_issues = {}
        file_issues = {}
        
        for issue in results['issues']:
            severity = issue.get('severity', 'unknown')
            issue_counts[severity] = issue_counts.get(severity, 0) + 1
            
            # Extract component
            filepath = issue.get('file', '')
            if 'selforg' in filepath:
                component = 'selforg'
            elif 'ode_robots' in filepath:
                component = 'ode_robots'
            elif 'ga_tools' in filepath:
                component = 'ga_tools'
            elif 'opende' in filepath:
                component = 'opende'
            else:
                component = 'other'
                
            if component not in component_issues:
                component_issues[component] = {}
            component_issues[component][severity] = component_issues[component].get(severity, 0) + 1
            
            # Count by file
            file_issues[filepath] = file_issues.get(filepath, 0) + 1
        
        # Get top files with most issues
        top_files = sorted(file_issues.items(), key=lambda x: x[1], reverse=True)[:10]
        
        html_content = f"""
<!DOCTYPE html>
<html>
<head>
    <title>LPZRobots Cppcheck Analysis - {self.profile_name}</title>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        * {{ box-sizing: border-box; margin: 0; padding: 0; }}
        body {{ 
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background: #f5f7fa;
            color: #1a202c;
            line-height: 1.6;
        }}
        .container {{
            max-width: 1400px;
            margin: 0 auto;
            padding: 20px;
        }}
        .header {{
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            border-radius: 16px;
            margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
        }}
        .header h1 {{
            font-size: 2.5em;
            font-weight: 700;
            margin-bottom: 10px;
        }}
        .header .profile {{
            display: inline-block;
            background: rgba(255,255,255,0.2);
            padding: 8px 16px;
            border-radius: 20px;
            margin-top: 10px;
        }}
        .stats-overview {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }}
        .stat-box {{
            background: white;
            padding: 25px;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
            text-align: center;
            transition: transform 0.2s;
        }}
        .stat-box:hover {{
            transform: translateY(-2px);
            box-shadow: 0 8px 15px rgba(0,0,0,0.1);
        }}
        .stat-value {{
            font-size: 2.5em;
            font-weight: 700;
            margin: 10px 0;
        }}
        .stat-label {{
            font-size: 0.9em;
            color: #718096;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }}
        .chart-section {{
            background: white;
            padding: 30px;
            border-radius: 12px;
            margin-bottom: 30px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }}
        .chart {{
            width: 100%;
            height: 400px;
            margin: 20px 0;
        }}
        .issues-section {{
            background: white;
            padding: 30px;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }}
        .severity-group {{
            margin-bottom: 30px;
        }}
        .severity-header {{
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 15px;
            padding-bottom: 10px;
            border-bottom: 2px solid #e2e8f0;
        }}
        .severity-title {{
            font-size: 1.4em;
            font-weight: 600;
        }}
        .severity-count {{
            background: #e2e8f0;
            padding: 4px 12px;
            border-radius: 20px;
            font-weight: 500;
        }}
        .issue {{
            margin: 10px 0;
            padding: 15px;
            border-left: 4px solid;
            border-radius: 4px;
            background: #f7fafc;
            transition: background 0.2s;
        }}
        .issue:hover {{
            background: #edf2f7;
        }}
        .issue.error {{ 
            border-color: #f56565;
            background: #fff5f5;
        }}
        .issue.warning {{ 
            border-color: #ed8936;
            background: #fffaf0;
        }}
        .issue.style {{ 
            border-color: #4299e1;
            background: #ebf8ff;
        }}
        .issue.performance {{ 
            border-color: #48bb78;
            background: #f0fff4;
        }}
        .issue-file {{
            font-weight: 600;
            color: #2d3748;
            margin-bottom: 5px;
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
        }}
        .issue-message {{
            color: #4a5568;
            line-height: 1.5;
        }}
        .issue-id {{
            display: inline-block;
            background: rgba(0,0,0,0.1);
            padding: 2px 8px;
            border-radius: 4px;
            font-size: 0.85em;
            margin-left: 10px;
            font-family: monospace;
        }}
        .top-files {{
            margin-top: 20px;
            padding: 20px;
            background: #f7fafc;
            border-radius: 8px;
        }}
        .file-item {{
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid #e2e8f0;
        }}
        .file-item:last-child {{
            border-bottom: none;
        }}
        .collapse-btn {{
            background: #4299e1;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
            cursor: pointer;
            font-size: 0.9em;
            margin-top: 10px;
        }}
        .collapse-btn:hover {{
            background: #3182ce;
        }}
        .collapsed {{
            display: none;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Cppcheck Analysis Report</h1>
            <div class="profile">Profile: {self.profile_name}</div>
            <p style="margin-top: 10px; opacity: 0.9;">Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
        </div>
        
        <div class="stats-overview">
            <div class="stat-box">
                <div class="stat-label">Total Issues</div>
                <div class="stat-value" style="color: #805ad5;">{len(results['issues']):,}</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">Errors</div>
                <div class="stat-value" style="color: #f56565;">{issue_counts.get('error', 0):,}</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">Warnings</div>
                <div class="stat-value" style="color: #ed8936;">{issue_counts.get('warning', 0):,}</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">Style Issues</div>
                <div class="stat-value" style="color: #4299e1;">{issue_counts.get('style', 0):,}</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">Performance</div>
                <div class="stat-value" style="color: #48bb78;">{issue_counts.get('performance', 0):,}</div>
            </div>
            <div class="stat-box">
                <div class="stat-label">Analysis Time</div>
                <div class="stat-value" style="color: #667eea;">{results['stats'].get('analysis_time', 0):.1f}s</div>
            </div>
        </div>
        
        <div class="chart-section">
            <h2>📊 Issue Distribution</h2>
            <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 30px;">
                <div id="severityChart" class="chart" style="height: 300px;"></div>
                <div id="componentChart" class="chart" style="height: 300px;"></div>
            </div>
            
            <div class="top-files">
                <h3>📁 Top Files with Issues</h3>
"""
        
        for filepath, count in top_files:
            short_path = filepath.replace(str(PROJECT_ROOT) + '/', '')
            html_content += f"""
                <div class="file-item">
                    <span style="font-family: monospace; font-size: 0.9em;">{short_path}</span>
                    <span style="font-weight: 600; color: #805ad5;">{count} issues</span>
                </div>
"""
        
        html_content += """
            </div>
        </div>
        
        <div class="issues-section">
            <h2>🔍 Issues Details</h2>
"""
        
        # Group issues by severity
        issues_by_severity = {}
        for issue in results['issues']:
            severity = issue.get('severity', 'unknown')
            if severity not in issues_by_severity:
                issues_by_severity[severity] = []
            issues_by_severity[severity].append(issue)
        
        # Display issues by severity
        severity_order = ['error', 'warning', 'performance', 'style', 'information', 'portability']
        severity_colors = {
            'error': '#f56565',
            'warning': '#ed8936',
            'style': '#4299e1',
            'performance': '#48bb78',
            'information': '#805ad5',
            'portability': '#d69e2e'
        }
        
        for severity in severity_order:
            if severity in issues_by_severity:
                issues = issues_by_severity[severity]
                color = severity_colors.get(severity, '#718096')
                
                html_content += f"""
            <div class="severity-group">
                <div class="severity-header">
                    <h3 class="severity-title" style="color: {color};">{severity.capitalize()}s</h3>
                    <span class="severity-count">{len(issues)}</span>
                </div>
                <div id="{severity}-issues">
"""
                
                # Show first 10 issues, hide the rest
                for i, issue in enumerate(issues[:10]):
                    filepath = issue.get('file', 'unknown').replace(str(PROJECT_ROOT) + '/', '')
                    issue_id = issue.get('id', '')
                    
                    html_content += f"""
                    <div class="issue {severity}">
                        <div class="issue-file">
                            {filepath}:{issue.get('line', '0')}
                            {f'<span class="issue-id">{issue_id}</span>' if issue_id else ''}
                        </div>
                        <div class="issue-message">{issue.get('message', '')}</div>
                    </div>
"""
                
                if len(issues) > 10:
                    html_content += f"""
                    <div id="{severity}-collapsed" class="collapsed">
"""
                    for issue in issues[10:]:
                        filepath = issue.get('file', 'unknown').replace(str(PROJECT_ROOT) + '/', '')
                        issue_id = issue.get('id', '')
                        
                        html_content += f"""
                        <div class="issue {severity}">
                            <div class="issue-file">
                                {filepath}:{issue.get('line', '0')}
                                {f'<span class="issue-id">{issue_id}</span>' if issue_id else ''}
                            </div>
                            <div class="issue-message">{issue.get('message', '')}</div>
                        </div>
"""
                    
                    html_content += f"""
                    </div>
                    <button class="collapse-btn" onclick="toggleCollapse('{severity}')">
                        Show {len(issues) - 10} more {severity}s
                    </button>
"""
                
                html_content += """
                </div>
            </div>
"""
        
        # Add charts JavaScript
        html_content += """
        </div>
    </div>
    
    <script>
        // Severity distribution pie chart
        var severityData = [{
            values: [""" + ', '.join(str(issue_counts.get(s, 0)) for s in ['error', 'warning', 'style', 'performance']) + """],
            labels: ['Errors', 'Warnings', 'Style', 'Performance'],
            type: 'pie',
            hole: .4,
            marker: {
                colors: ['#f56565', '#ed8936', '#4299e1', '#48bb78']
            },
            textinfo: 'label+percent',
            textposition: 'outside'
        }];
        
        var pieLayout = {
            title: 'Issues by Severity',
            showlegend: false,
            height: 300,
            margin: { t: 40, b: 20, l: 20, r: 20 },
            plot_bgcolor: 'rgba(0,0,0,0)',
            paper_bgcolor: 'rgba(0,0,0,0)',
            font: { family: 'Inter, sans-serif' }
        };
        
        Plotly.newPlot('severityChart', severityData, pieLayout, {responsive: true});
        
        // Component distribution bar chart
        var componentData = [{
            x: [""" + ', '.join(f'"{c}"' for c in component_issues.keys()) + """],
            y: [""" + ', '.join(str(sum(v.values())) for v in component_issues.values()) + """],
            type: 'bar',
            marker: {
                color: '#667eea'
            }
        }];
        
        var barLayout = {
            title: 'Issues by Component',
            xaxis: { title: '' },
            yaxis: { title: 'Number of Issues' },
            height: 300,
            margin: { t: 40, b: 60, l: 60, r: 20 },
            plot_bgcolor: 'rgba(0,0,0,0)',
            paper_bgcolor: 'rgba(0,0,0,0)',
            font: { family: 'Inter, sans-serif' }
        };
        
        Plotly.newPlot('componentChart', componentData, barLayout, {responsive: true});
        
        // Toggle collapse function
        function toggleCollapse(severity) {
            var collapsed = document.getElementById(severity + '-collapsed');
            var btn = event.target;
            
            if (collapsed.classList.contains('collapsed')) {
                collapsed.classList.remove('collapsed');
                btn.textContent = 'Show less';
            } else {
                collapsed.classList.add('collapsed');
                btn.textContent = btn.textContent.replace('Show less', 'Show ' + btn.textContent.match(/\\d+/)[0] + ' more ' + severity + 's');
            }
        }
    </script>
</body>
</html>
"""
        
        with open(output_path, 'w') as f:
            f.write(html_content)

def main():
    parser = argparse.ArgumentParser(description='LPZRobots Cppcheck Analysis Engine')
    parser.add_argument('--profile', '-p', default='quick_check',
                        help='Analysis profile to use')
    parser.add_argument('--list-profiles', action='store_true',
                        help='List available profiles')
    parser.add_argument('--incremental', '-i', action='store_true',
                        help='Run incremental analysis on changed files only')
    parser.add_argument('--files', '-f', nargs='+',
                        help='Specific files to analyze')
    parser.add_argument('--format', choices=['text', 'json', 'html'], default='text',
                        help='Output format for report')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Verbose output')
    parser.add_argument('--threshold', type=int, default=0,
                        help='Error threshold for CI/CD (0 = no threshold)')
    
    args = parser.parse_args()
    
    # List profiles if requested
    if args.list_profiles:
        print(f"{Colors.BLUE}Available profiles:{Colors.NC}")
        for profile_file in PROFILES_DIR.glob('*.json'):
            with open(profile_file) as f:
                profile = json.load(f)
                print(f"  {profile_file.stem}: {profile.get('description', 'No description')}")
        return 0
        
    # Create necessary directories
    CACHE_DIR.mkdir(parents=True, exist_ok=True)
    REPORTS_DIR.mkdir(parents=True, exist_ok=True)
    
    # Run analysis
    analyzer = CppcheckAnalyzer(args.profile, args.verbose)
    results = analyzer.analyze(args.incremental, args.files)
    
    # Generate report
    report_path = analyzer.generate_report(results, args.format)
    
    # Print summary
    print(f"\n{Colors.BLUE}Analysis Summary:{Colors.NC}")
    print(f"  Files analyzed: {results['stats']['files_analyzed']}")
    print(f"  Errors: {Colors.RED}{results['stats']['errors']}{Colors.NC}")
    print(f"  Warnings: {Colors.YELLOW}{results['stats']['warnings']}{Colors.NC}")
    print(f"  Style issues: {results['stats']['style']}")
    print(f"  Performance issues: {results['stats']['performance']}")
    print(f"  Analysis time: {results['stats'].get('analysis_time', 0):.2f}s")
    
    # Check threshold for CI/CD
    if args.threshold > 0 and results['stats']['errors'] > args.threshold:
        print(f"\n{Colors.RED}Error threshold exceeded! ({results['stats']['errors']} > {args.threshold}){Colors.NC}")
        return 1
        
    return 0

if __name__ == '__main__':
    sys.exit(main())