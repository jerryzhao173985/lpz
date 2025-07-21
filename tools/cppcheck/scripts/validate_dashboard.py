#!/usr/bin/env python3
"""
Validate Ultimate Dashboard Functionality
Tests all features of the generated HTML report
"""

import json
import sys
from pathlib import Path
from html.parser import HTMLParser
import re

class DashboardValidator(HTMLParser):
    def __init__(self):
        super().__init__()
        self.features = {
            'quality_score': False,
            'executive_summary': False,
            'interactive_charts': False,
            'issue_explorer': False,
            'code_modal': False,
            'search_functionality': False,
            'pagination': False,
            'collapsible_sections': False,
            'fix_suggestions': False,
            'analytics_tabs': False,
            'font_awesome_icons': False,
            'no_encoding_issues': True
        }
        self.in_script = False
        self.script_content = []
        self.issues_count = 0
        self.charts_found = []
        
    def handle_starttag(self, tag, attrs):
        attrs_dict = dict(attrs)
        
        # Check for quality score
        if tag == 'div' and 'quality-score' in attrs_dict.get('class', ''):
            self.features['quality_score'] = True
            
        # Check for executive summary (as part of dashboard content)
        if tag == 'div' and 'executive-dashboard' in attrs_dict.get('class', ''):
            self.features['executive_summary'] = True
            
        # Check for issue explorer
        if tag == 'div' and 'issue-explorer' in attrs_dict.get('class', ''):
            self.features['issue_explorer'] = True
            
        # Check for code modal
        if tag == 'div' and attrs_dict.get('id') == 'codeModal':
            self.features['code_modal'] = True
            
        # Check for search input
        if tag == 'input' and attrs_dict.get('id') == 'searchInput':
            self.features['search_functionality'] = True
            
        # Check for pagination
        if tag == 'div' and 'pagination' in attrs_dict.get('class', ''):
            self.features['pagination'] = True
            
        # Check for analytics tabs
        if tag == 'div' and 'insight-tab' in attrs_dict.get('class', ''):
            self.features['analytics_tabs'] = True
            
        # Check for Font Awesome
        if tag == 'link' and 'font-awesome' in attrs_dict.get('href', ''):
            self.features['font_awesome_icons'] = True
            
        # Check for charts
        if tag == 'div' and attrs_dict.get('id') and 'Chart' in attrs_dict.get('id', ''):
            self.charts_found.append(attrs_dict['id'])
            
        # Count issue rows
        if tag == 'tr' and attrs_dict.get('class') == 'issue-row':
            self.issues_count += 1
            
        if tag == 'script':
            self.in_script = True
            
    def handle_endtag(self, tag):
        if tag == 'script':
            self.in_script = False
            
    def handle_data(self, data):
        if self.in_script:
            self.script_content.append(data)
            
        # Check for encoding issues
        if any(ord(char) > 127 and ord(char) < 160 for char in data):
            self.features['no_encoding_issues'] = False
            
    def validate(self):
        # Check for interactive charts
        script_text = '\n'.join(self.script_content)
        if 'Plotly.newPlot' in script_text or 'new Chart' in script_text:
            self.features['interactive_charts'] = True
            
        # Check for collapsible sections
        if 'toggleSection' in script_text or 'toggleCollapse' in script_text:
            self.features['collapsible_sections'] = True
            
        # Check for fix suggestions
        if 'fix-suggestion' in script_text or 'getFix' in script_text:
            self.features['fix_suggestions'] = True
            
        return self.features

def validate_report(report_path: Path):
    """Validate a generated HTML report"""
    print(f"Validating dashboard: {report_path}")
    print("=" * 60)
    
    # Check file exists
    if not report_path.exists():
        print(f"❌ Report file not found: {report_path}")
        return False
        
    # Check JSON data exists
    json_path = report_path.parent / 'report.json'
    if not json_path.exists():
        print(f"❌ JSON data file not found: {json_path}")
        return False
        
    # Load JSON data
    with open(json_path) as f:
        data = json.load(f)
        
    print(f"📊 Issues found: {len(data['issues'])}")
    print(f"   - Errors: {data['stats'].get('errors', 0)}")
    print(f"   - Warnings: {data['stats'].get('warnings', 0)}")
    print(f"   - Style: {data['stats'].get('style', 0)}")
    print(f"   - Performance: {data['stats'].get('performance', 0)}")
    print()
    
    # Parse HTML
    with open(report_path, 'r', encoding='utf-8') as f:
        html_content = f.read()
        
    validator = DashboardValidator()
    validator.feed(html_content)
    features = validator.validate()
    
    print("🔍 Feature Validation:")
    all_good = True
    for feature, found in features.items():
        status = "✅" if found else "❌"
        print(f"   {status} {feature.replace('_', ' ').title()}")
        if not found:
            all_good = False
            
    print()
    print(f"📈 Charts found: {len(validator.charts_found)}")
    for chart in validator.charts_found[:5]:
        print(f"   - {chart}")
        
    print()
    print(f"📝 Issue rows in explorer: {validator.issues_count}")
    
    # Check file size
    file_size = report_path.stat().st_size / 1024 / 1024  # MB
    print(f"📁 Report size: {file_size:.2f} MB")
    
    if file_size > 10:
        print("   ⚠️  Large report size may affect performance")
        
    print()
    if all_good:
        print("✅ All dashboard features validated successfully!")
    else:
        print("❌ Some features are missing or not working properly")
        
    return all_good

def main():
    if len(sys.argv) < 2:
        # Find the latest report
        reports_dir = Path(__file__).parent.parent / 'reports'
        latest_report = None
        latest_time = 0
        
        for profile_dir in reports_dir.glob('*'):
            if profile_dir.is_dir():
                for report_dir in profile_dir.glob('*'):
                    if report_dir.is_dir():
                        html_file = report_dir / 'report.html'
                        if html_file.exists():
                            mtime = html_file.stat().st_mtime
                            if mtime > latest_time:
                                latest_time = mtime
                                latest_report = html_file
                                
        if latest_report:
            print(f"Using latest report: {latest_report}")
            validate_report(latest_report)
        else:
            print("No reports found to validate")
            sys.exit(1)
    else:
        report_path = Path(sys.argv[1])
        validate_report(report_path)

if __name__ == '__main__':
    sys.exit(main())
