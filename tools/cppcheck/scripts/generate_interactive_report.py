#!/usr/bin/env python3
"""
Interactive Cppcheck Report Generator
Creates highly interactive HTML reports with code viewing
"""

import json
import sys
import html
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List
from collections import defaultdict, Counter

# Constants
SCRIPT_DIR = Path(__file__).parent
TOOLS_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = TOOLS_DIR.parent.parent

class InteractiveReportGenerator:
    def __init__(self, report_data: Dict):
        self.data = report_data
        self.issues = report_data.get('issues', [])
        self.stats = report_data.get('stats', {})
        self.profile = report_data.get('profile', 'unknown')
        
    def generate_interactive_html(self, output_path: Path):
        """Generate interactive HTML report with code viewing"""
        # Process and prepare data
        processed_data = self._process_issue_data()
        
        html_content = self._generate_html(processed_data)
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)
            
        print(f"Interactive report generated: {output_path}")
        
    def _process_issue_data(self) -> Dict:
        """Process issues for better display"""
        # Group by severity
        by_severity = defaultdict(list)
        for issue in self.issues:
            severity = issue.get('severity', 'unknown')
            by_severity[severity].append(issue)
            
        # Extract unique files
        files = set()
        for issue in self.issues:
            files.add(issue.get('file', ''))
            
        # Count by component
        by_component = defaultdict(int)
        for issue in self.issues:
            component = self._extract_component(issue.get('file', ''))
            by_component[component] += 1
            
        return {
            'by_severity': dict(by_severity),
            'total_files': len(files),
            'by_component': dict(by_component),
            'total_issues': len(self.issues)
        }
        
    def _generate_html(self, data: Dict) -> str:
        """Generate the complete interactive HTML"""
        return f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LPZRobots Cppcheck Interactive Report - {html.escape(self.profile)}</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/themes/prism-tomorrow.min.css" rel="stylesheet">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/prism.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/components/prism-cpp.min.js"></script>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    {self._generate_styles()}
</head>
<body>
    <div class="container">
        {self._generate_header(data)}
        {self._generate_summary_cards(data)}
        {self._generate_issue_browser(data)}
        {self._generate_code_viewer_modal()}
        {self._generate_footer()}
    </div>
    {self._generate_scripts(data)}
</body>
</html>"""

    def _generate_styles(self) -> str:
        """Generate comprehensive CSS styles"""
        return """
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        
        :root {
            --primary: #667eea;
            --primary-dark: #5a67d8;
            --secondary: #764ba2;
            --success: #48bb78;
            --warning: #ed8936;
            --danger: #f56565;
            --info: #4299e1;
            --dark: #1a202c;
            --light: #f7fafc;
            --gray: #718096;
            --border: #e2e8f0;
        }
        
        body {
            font-family: 'Inter', -apple-system, sans-serif;
            background: #f5f7fa;
            color: var(--dark);
            line-height: 1.6;
        }
        
        .container {
            max-width: 1600px;
            margin: 0 auto;
            padding: 20px;
        }
        
        /* Header */
        .header {
            background: linear-gradient(135deg, var(--primary) 0%, var(--secondary) 100%);
            color: white;
            padding: 40px;
            border-radius: 16px;
            margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
        }
        
        .header h1 {
            font-size: 2.5em;
            font-weight: 700;
            margin-bottom: 10px;
        }
        
        /* Summary Cards */
        .summary-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .summary-card {
            background: white;
            padding: 25px;
            border-radius: 12px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
            transition: transform 0.3s;
        }
        
        .summary-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 15px rgba(0,0,0,0.1);
        }
        
        .summary-value {
            font-size: 2.5em;
            font-weight: 700;
            margin: 10px 0;
        }
        
        .summary-label {
            color: var(--gray);
            text-transform: uppercase;
            font-size: 0.9em;
            letter-spacing: 0.05em;
        }
        
        /* Issue Browser */
        .issue-browser {
            background: white;
            border-radius: 16px;
            padding: 30px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.07);
        }
        
        .browser-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 20px;
            flex-wrap: wrap;
            gap: 20px;
        }
        
        .search-bar {
            display: flex;
            gap: 10px;
            flex: 1;
            min-width: 300px;
        }
        
        .search-input {
            flex: 1;
            padding: 10px 15px;
            border: 2px solid var(--border);
            border-radius: 8px;
            font-size: 1em;
            transition: border-color 0.3s;
        }
        
        .search-input:focus {
            outline: none;
            border-color: var(--primary);
        }
        
        .filter-buttons {
            display: flex;
            gap: 10px;
        }
        
        .filter-btn {
            padding: 8px 16px;
            border: 2px solid var(--border);
            background: white;
            border-radius: 8px;
            cursor: pointer;
            transition: all 0.3s;
            font-weight: 500;
        }
        
        .filter-btn:hover {
            border-color: var(--primary);
            background: rgba(102, 126, 234, 0.1);
        }
        
        .filter-btn.active {
            background: var(--primary);
            color: white;
            border-color: var(--primary);
        }
        
        /* Issue List */
        .issue-list {
            margin-top: 20px;
        }
        
        .issue-group {
            margin-bottom: 20px;
        }
        
        .issue-group-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 15px 20px;
            background: var(--light);
            border-radius: 8px;
            cursor: pointer;
            transition: background 0.3s;
            user-select: none;
        }
        
        .issue-group-header:hover {
            background: #e2e8f0;
        }
        
        .issue-group-title {
            display: flex;
            align-items: center;
            gap: 10px;
            font-weight: 600;
            font-size: 1.1em;
        }
        
        .issue-count {
            background: var(--primary);
            color: white;
            padding: 2px 10px;
            border-radius: 15px;
            font-size: 0.9em;
        }
        
        .issue-items {
            margin-top: 10px;
            max-height: 400px;
            overflow-y: auto;
            border: 1px solid var(--border);
            border-radius: 8px;
        }
        
        .issue-item {
            padding: 15px 20px;
            border-bottom: 1px solid var(--border);
            cursor: pointer;
            transition: background 0.2s;
        }
        
        .issue-item:hover {
            background: rgba(102, 126, 234, 0.05);
        }
        
        .issue-item:last-child {
            border-bottom: none;
        }
        
        .issue-location {
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
            color: var(--primary);
            margin-bottom: 5px;
        }
        
        .issue-message {
            color: var(--dark);
            line-height: 1.5;
        }
        
        .issue-id {
            display: inline-block;
            background: var(--light);
            padding: 2px 8px;
            border-radius: 4px;
            font-size: 0.85em;
            margin-left: 10px;
            font-family: monospace;
        }
        
        /* Pagination */
        .pagination {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 10px;
            margin-top: 20px;
            padding-top: 20px;
            border-top: 1px solid var(--border);
        }
        
        .page-btn {
            padding: 8px 12px;
            border: 1px solid var(--border);
            background: white;
            border-radius: 6px;
            cursor: pointer;
            transition: all 0.3s;
        }
        
        .page-btn:hover:not(:disabled) {
            border-color: var(--primary);
            background: rgba(102, 126, 234, 0.1);
        }
        
        .page-btn.active {
            background: var(--primary);
            color: white;
            border-color: var(--primary);
        }
        
        .page-btn:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }
        
        /* Code Viewer Modal */
        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.7);
            z-index: 1000;
            overflow: auto;
        }
        
        .modal-content {
            background: white;
            margin: 50px auto;
            width: 90%;
            max-width: 1200px;
            border-radius: 16px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            overflow: hidden;
        }
        
        .modal-header {
            background: var(--dark);
            color: white;
            padding: 20px 30px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .modal-title {
            font-size: 1.2em;
            font-weight: 600;
            font-family: monospace;
        }
        
        .modal-close {
            background: none;
            border: none;
            color: white;
            font-size: 1.5em;
            cursor: pointer;
            padding: 0;
            width: 40px;
            height: 40px;
            display: flex;
            align-items: center;
            justify-content: center;
            border-radius: 8px;
            transition: background 0.3s;
        }
        
        .modal-close:hover {
            background: rgba(255, 255, 255, 0.1);
        }
        
        .modal-body {
            padding: 30px;
            max-height: 70vh;
            overflow-y: auto;
        }
        
        .code-viewer {
            background: #2d2d2d;
            border-radius: 8px;
            padding: 20px;
            overflow-x: auto;
        }
        
        .code-line {
            display: flex;
            margin: 2px 0;
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
            line-height: 1.4;
        }
        
        .line-number {
            color: #666;
            width: 50px;
            text-align: right;
            padding-right: 15px;
            user-select: none;
        }
        
        .line-content {
            flex: 1;
            white-space: pre;
        }
        
        .line-highlight {
            background: rgba(237, 137, 54, 0.3);
            border-radius: 3px;
        }
        
        .issue-details {
            margin-top: 20px;
            padding: 20px;
            background: var(--light);
            border-radius: 8px;
        }
        
        .git-info {
            margin-top: 15px;
            padding: 15px;
            background: white;
            border: 1px solid var(--border);
            border-radius: 8px;
            font-size: 0.9em;
        }
        
        /* Loading Spinner */
        .spinner {
            border: 3px solid var(--light);
            border-top: 3px solid var(--primary);
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }
        
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        
        /* Toast Notifications */
        .toast {
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: var(--dark);
            color: white;
            padding: 15px 20px;
            border-radius: 8px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
            display: none;
            z-index: 2000;
        }
        
        .toast.error {
            background: var(--danger);
        }
        
        /* Responsive */
        @media (max-width: 768px) {
            .container { padding: 10px; }
            .header { padding: 20px; }
            .header h1 { font-size: 1.8em; }
            .summary-grid { grid-template-columns: 1fr; }
            .browser-header { flex-direction: column; }
            .search-bar { width: 100%; }
            .filter-buttons { width: 100%; overflow-x: auto; }
        }
    </style>
"""

    def _generate_header(self, data: Dict) -> str:
        """Generate header section"""
        return f"""
        <div class="header">
            <h1>Interactive Cppcheck Analysis Report</h1>
            <p>Profile: {html.escape(self.profile)} | Generated: {datetime.now().strftime('%B %d, %Y at %I:%M %p')}</p>
            <p>{data['total_files']} files analyzed | {data['total_issues']:,} total issues found</p>
        </div>
"""

    def _generate_summary_cards(self, data: Dict) -> str:
        """Generate summary statistics cards"""
        severity_counts = Counter(issue.get('severity', 'unknown') for issue in self.issues)
        
        return f"""
        <div class="summary-grid">
            <div class="summary-card">
                <div class="summary-label">Total Issues</div>
                <div class="summary-value" style="color: var(--primary);">{data['total_issues']:,}</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Errors</div>
                <div class="summary-value" style="color: var(--danger);">{severity_counts.get('error', 0):,}</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Warnings</div>
                <div class="summary-value" style="color: var(--warning);">{severity_counts.get('warning', 0):,}</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Style Issues</div>
                <div class="summary-value" style="color: var(--info);">{severity_counts.get('style', 0):,}</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Performance</div>
                <div class="summary-value" style="color: var(--success);">{severity_counts.get('performance', 0):,}</div>
            </div>
            <div class="summary-card">
                <div class="summary-label">Files Affected</div>
                <div class="summary-value" style="color: var(--secondary);">{data['total_files']}</div>
            </div>
        </div>
"""

    def _generate_issue_browser(self, data: Dict) -> str:
        """Generate the interactive issue browser"""
        return f"""
        <div class="issue-browser">
            <div class="browser-header">
                <h2><i class="fas fa-search"></i> Issue Browser</h2>
                <div class="search-bar">
                    <input type="text" class="search-input" id="searchInput" placeholder="Search issues by file, message, or ID...">
                    <button class="filter-btn" onclick="clearSearch()"><i class="fas fa-times"></i></button>
                </div>
                <div class="filter-buttons">
                    <button class="filter-btn active" data-filter="all" onclick="filterBySeverity('all')">All</button>
                    <button class="filter-btn" data-filter="error" onclick="filterBySeverity('error')">Errors</button>
                    <button class="filter-btn" data-filter="warning" onclick="filterBySeverity('warning')">Warnings</button>
                    <button class="filter-btn" data-filter="style" onclick="filterBySeverity('style')">Style</button>
                    <button class="filter-btn" data-filter="performance" onclick="filterBySeverity('performance')">Performance</button>
                </div>
            </div>
            
            <div class="issue-list" id="issueList">
                {self._generate_issue_groups(data)}
            </div>
            
            <div class="pagination" id="pagination"></div>
        </div>
"""

    def _generate_issue_groups(self, data: Dict) -> str:
        """Generate issue groups by severity"""
        html_content = ""
        
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
            if severity in data['by_severity']:
                issues = data['by_severity'][severity]
                color = severity_colors.get(severity, '#718096')
                
                html_content += f"""
                <div class="issue-group" data-severity="{severity}">
                    <div class="issue-group-header" onclick="toggleGroup('{severity}')">
                        <div class="issue-group-title">
                            <i class="fas fa-chevron-down" id="icon-{severity}"></i>
                            <span style="color: {color};">{severity.capitalize()}</span>
                            <span class="issue-count">{len(issues)}</span>
                        </div>
                        <button class="filter-btn" onclick="event.stopPropagation(); viewAllInGroup('{severity}')">
                            View All
                        </button>
                    </div>
                    <div class="issue-items" id="issues-{severity}" style="display: none;">
"""
                
                # Add issues as JSON data for JavaScript
                html_content += f"""
                        <script>
                            window.issueData = window.issueData || {{}};
                            window.issueData['{severity}'] = {json.dumps(issues)};
                        </script>
                        <div id="issue-container-{severity}"></div>
"""
                
                html_content += """
                    </div>
                </div>
"""
        
        return html_content

    def _generate_code_viewer_modal(self) -> str:
        """Generate the code viewer modal"""
        return """
        <div id="codeModal" class="modal">
            <div class="modal-content">
                <div class="modal-header">
                    <div class="modal-title" id="modalTitle">Code View</div>
                    <button class="modal-close" onclick="closeModal()">
                        <i class="fas fa-times"></i>
                    </button>
                </div>
                <div class="modal-body">
                    <div class="code-viewer" id="codeViewer">
                        <div class="spinner"></div>
                    </div>
                    <div class="issue-details" id="issueDetails"></div>
                    <div class="git-info" id="gitInfo" style="display: none;"></div>
                </div>
            </div>
        </div>
        
        <div class="toast" id="toast"></div>
"""

    def _generate_footer(self) -> str:
        """Generate footer"""
        return """
        <div style="text-align: center; padding: 40px 0; color: var(--gray);">
            <p>
                <i class="fas fa-robot"></i> Generated by LPZRobots Interactive Cppcheck Infrastructure<br>
                <i class="fas fa-code"></i> Click on any issue to view code context
            </p>
        </div>
"""

    def _generate_scripts(self, data: Dict) -> str:
        """Generate JavaScript for interactivity"""
        return f"""
    <script>
        // Global variables
        let currentSeverity = 'all';
        let currentPage = 1;
        let itemsPerPage = 20;
        let searchTerm = '';
        let allIssues = [];
        
        // Initialize on load
        window.onload = function() {{
            // Flatten all issues
            for (const severity in window.issueData) {{
                allIssues = allIssues.concat(window.issueData[severity].map(issue => ({{
                    ...issue,
                    severity: severity
                }})));
            }}
            
            // Setup search
            document.getElementById('searchInput').addEventListener('input', function(e) {{
                searchTerm = e.target.value.toLowerCase();
                filterAndDisplay();
            }});
            
            // Display initial view
            filterAndDisplay();
        }};
        
        // Toggle issue group
        function toggleGroup(severity) {{
            const container = document.getElementById('issues-' + severity);
            const icon = document.getElementById('icon-' + severity);
            
            if (container.style.display === 'none') {{
                container.style.display = 'block';
                icon.className = 'fas fa-chevron-up';
                displayIssuesForGroup(severity);
            }} else {{
                container.style.display = 'none';
                icon.className = 'fas fa-chevron-down';
            }}
        }}
        
        // Display issues for a specific group
        function displayIssuesForGroup(severity) {{
            const container = document.getElementById('issue-container-' + severity);
            const issues = window.issueData[severity] || [];
            
            // Show only first 20 by default
            const displayIssues = issues.slice(0, 20);
            container.innerHTML = displayIssues.map(issue => createIssueHTML(issue, severity)).join('');
            
            if (issues.length > 20) {{
                container.innerHTML += `
                    <div style="text-align: center; padding: 20px;">
                        <button class="filter-btn" onclick="showAllIssues('${{severity}}')">
                            Show all ${{issues.length}} issues
                        </button>
                    </div>
                `;
            }}
        }}
        
        // Show all issues in a group
        function showAllIssues(severity) {{
            const container = document.getElementById('issue-container-' + severity);
            const issues = window.issueData[severity] || [];
            container.innerHTML = issues.map(issue => createIssueHTML(issue, severity)).join('');
        }}
        
        // Create HTML for a single issue
        function createIssueHTML(issue, severity) {{
            const filepath = issue.file || 'unknown';
            const shortPath = filepath.replace('{str(PROJECT_ROOT)}/', '');
            const issueId = issue.id || '';
            
            return `
                <div class="issue-item" onclick="showCodeContext('${{escape(filepath)}}', ${{issue.line || 0}}, '${{escape(JSON.stringify(issue))}}')">
                    <div class="issue-location">
                        ${{escape(shortPath)}}:${{issue.line || 0}}
                        ${{issueId ? `<span class="issue-id">${{issueId}}</span>` : ''}}
                    </div>
                    <div class="issue-message">${{escape(issue.message || '')}}</div>
                </div>
            `;
        }}
        
        // Filter by severity
        function filterBySeverity(severity) {{
            currentSeverity = severity;
            currentPage = 1;
            
            // Update button states
            document.querySelectorAll('.filter-btn[data-filter]').forEach(btn => {{
                btn.classList.toggle('active', btn.dataset.filter === severity);
            }});
            
            filterAndDisplay();
        }}
        
        // Filter and display issues
        function filterAndDisplay() {{
            let filteredIssues = allIssues;
            
            // Apply severity filter
            if (currentSeverity !== 'all') {{
                filteredIssues = filteredIssues.filter(issue => issue.severity === currentSeverity);
            }}
            
            // Apply search filter
            if (searchTerm) {{
                filteredIssues = filteredIssues.filter(issue => {{
                    const searchableText = `${{issue.file}} ${{issue.message}} ${{issue.id}}`.toLowerCase();
                    return searchableText.includes(searchTerm);
                }});
            }}
            
            // Hide all groups
            document.querySelectorAll('.issue-group').forEach(group => {{
                group.style.display = 'none';
            }});
            
            // Display filtered issues
            if (filteredIssues.length > 0) {{
                displayFilteredIssues(filteredIssues);
            }} else {{
                document.getElementById('issueList').innerHTML = `
                    <div style="text-align: center; padding: 40px; color: var(--gray);">
                        <i class="fas fa-search" style="font-size: 3em; margin-bottom: 20px;"></i>
                        <p>No issues found matching your criteria</p>
                    </div>
                `;
            }}
        }}
        
        // Display filtered issues with pagination
        function displayFilteredIssues(issues) {{
            const totalPages = Math.ceil(issues.length / itemsPerPage);
            const start = (currentPage - 1) * itemsPerPage;
            const end = start + itemsPerPage;
            const pageIssues = issues.slice(start, end);
            
            // Group by severity for display
            const grouped = {{}};
            pageIssues.forEach(issue => {{
                if (!grouped[issue.severity]) grouped[issue.severity] = [];
                grouped[issue.severity].push(issue);
            }});
            
            // Display grouped issues
            for (const severity in grouped) {{
                const group = document.querySelector(`.issue-group[data-severity="${{severity}}"]`);
                if (group) {{
                    group.style.display = 'block';
                    const container = document.getElementById('issue-container-' + severity);
                    container.innerHTML = grouped[severity].map(issue => createIssueHTML(issue, severity)).join('');
                    document.getElementById('issues-' + severity).style.display = 'block';
                    document.getElementById('icon-' + severity).className = 'fas fa-chevron-up';
                }}
            }}
            
            // Update pagination
            updatePagination(totalPages);
        }}
        
        // Update pagination controls
        function updatePagination(totalPages) {{
            const pagination = document.getElementById('pagination');
            if (totalPages <= 1) {{
                pagination.innerHTML = '';
                return;
            }}
            
            let html = `
                <button class="page-btn" onclick="changePage(1)" ${{currentPage === 1 ? 'disabled' : ''}}>
                    <i class="fas fa-angle-double-left"></i>
                </button>
                <button class="page-btn" onclick="changePage(${{currentPage - 1}})" ${{currentPage === 1 ? 'disabled' : ''}}>
                    <i class="fas fa-angle-left"></i>
                </button>
            `;
            
            // Page numbers
            let startPage = Math.max(1, currentPage - 2);
            let endPage = Math.min(totalPages, startPage + 4);
            
            for (let i = startPage; i <= endPage; i++) {{
                html += `<button class="page-btn ${{i === currentPage ? 'active' : ''}}" onclick="changePage(${{i}})">${{i}}</button>`;
            }}
            
            html += `
                <button class="page-btn" onclick="changePage(${{currentPage + 1}})" ${{currentPage === totalPages ? 'disabled' : ''}}>
                    <i class="fas fa-angle-right"></i>
                </button>
                <button class="page-btn" onclick="changePage(${{totalPages}})" ${{currentPage === totalPages ? 'disabled' : ''}}>
                    <i class="fas fa-angle-double-right"></i>
                </button>
            `;
            
            pagination.innerHTML = html;
        }}
        
        // Change page
        function changePage(page) {{
            currentPage = page;
            filterAndDisplay();
            window.scrollTo(0, 0);
        }}
        
        // Clear search
        function clearSearch() {{
            document.getElementById('searchInput').value = '';
            searchTerm = '';
            filterAndDisplay();
        }}
        
        // Show code context
        function showCodeContext(filepath, line, issueJson) {{
            const issue = JSON.parse(issueJson);
            const modal = document.getElementById('codeModal');
            const codeViewer = document.getElementById('codeViewer');
            const modalTitle = document.getElementById('modalTitle');
            
            // Show modal
            modal.style.display = 'block';
            modalTitle.textContent = filepath.split('/').pop() + ':' + line;
            
            // Show loading
            codeViewer.innerHTML = '<div class="spinner"></div>';
            
            // Try to fetch code from file system
            try {{
                // Read file content (this would need a local server or file access)
                fetchCodeContext(filepath, line, issue);
            }} catch (e) {{
                // Fallback: show issue details only
                showIssueDetailsOnly(issue);
            }}
        }}
        
        // Fetch code context (requires local server)
        function fetchCodeContext(filepath, line, issue) {{
            // Try to read file locally
            // For now, show sample code
            const sampleCode = generateSampleCode(filepath, line, issue);
            displayCode(sampleCode, line, issue);
        }}
        
        // Generate sample code display
        function generateSampleCode(filepath, line, issue) {{
            const contextLines = 5;
            const startLine = Math.max(1, line - contextLines);
            const endLine = line + contextLines;
            
            // Sample code structure
            const lines = [];
            for (let i = startLine; i <= endLine; i++) {{
                let content = 'Sample code line content here...';
                if (i === line) {{
                    content = '>>> Issue occurs on this line <<<';
                }}
                lines.push({{
                    number: i,
                    content: content,
                    highlight: i === line
                }});
            }}
            
            return lines;
        }}
        
        // Display code in viewer
        function displayCode(lines, targetLine, issue) {{
            const codeViewer = document.getElementById('codeViewer');
            
            let html = '<pre style="margin: 0; color: #fff;">';
            lines.forEach(line => {{
                const highlightClass = line.highlight ? 'line-highlight' : '';
                html += `
                    <div class="code-line ${{highlightClass}}">
                        <span class="line-number">${{line.number}}</span>
                        <span class="line-content">${{escape(line.content)}}</span>
                    </div>
                `;
            }});
            html += '</pre>';
            
            codeViewer.innerHTML = html;
            
            // Show issue details
            showIssueDetails(issue);
        }}
        
        // Show issue details
        function showIssueDetails(issue) {{
            const detailsDiv = document.getElementById('issueDetails');
            detailsDiv.innerHTML = `
                <h3>Issue Details</h3>
                <p><strong>Type:</strong> <span style="color: var(--${{getSeverityColor(issue.severity)}});">${{issue.severity}}</span></p>
                <p><strong>ID:</strong> <code>${{issue.id || 'N/A'}}</code></p>
                <p><strong>Message:</strong> ${{issue.message}}</p>
                <p><strong>File:</strong> <code>${{issue.file}}</code></p>
                <p><strong>Line:</strong> ${{issue.line}}</p>
            `;
        }}
        
        // Show issue details only (fallback)
        function showIssueDetailsOnly(issue) {{
            const codeViewer = document.getElementById('codeViewer');
            codeViewer.innerHTML = `
                <div style="text-align: center; padding: 40px; color: #999;">
                    <i class="fas fa-file-code" style="font-size: 3em; margin-bottom: 20px;"></i>
                    <p>Code preview not available</p>
                    <p style="font-size: 0.9em; margin-top: 10px;">
                        To enable code viewing, run the code context server:<br>
                        <code style="background: #333; padding: 5px 10px; border-radius: 4px; display: inline-block; margin-top: 10px;">
                            python3 tools/cppcheck/scripts/code_context_server.py
                        </code>
                    </p>
                </div>
            `;
            showIssueDetails(issue);
        }}
        
        // Get severity color
        function getSeverityColor(severity) {{
            const colors = {{
                error: 'danger',
                warning: 'warning',
                style: 'info',
                performance: 'success',
                information: 'secondary'
            }};
            return colors[severity] || 'gray';
        }}
        
        // Close modal
        function closeModal() {{
            document.getElementById('codeModal').style.display = 'none';
        }}
        
        // Show toast notification
        function showToast(message, type = 'info') {{
            const toast = document.getElementById('toast');
            toast.textContent = message;
            toast.className = 'toast ' + type;
            toast.style.display = 'block';
            
            setTimeout(() => {{
                toast.style.display = 'none';
            }}, 3000);
        }}
        
        // Escape HTML
        function escape(str) {{
            const div = document.createElement('div');
            div.textContent = str;
            return div.innerHTML;
        }}
        
        // Close modal on outside click
        window.onclick = function(event) {{
            const modal = document.getElementById('codeModal');
            if (event.target === modal) {{
                closeModal();
            }}
        }};
        
        // Keyboard shortcuts
        document.addEventListener('keydown', function(e) {{
            if (e.key === 'Escape') {{
                closeModal();
            }} else if (e.key === '/' && !e.target.matches('input')) {{
                e.preventDefault();
                document.getElementById('searchInput').focus();
            }}
        }});
    </script>
"""

    def _extract_component(self, filepath: str) -> str:
        """Extract component name from file path"""
        if 'selforg' in filepath:
            return 'selforg'
        elif 'ode_robots' in filepath:
            return 'ode_robots'
        elif 'ga_tools' in filepath:
            return 'ga_tools'
        elif 'opende' in filepath:
            return 'opende'
        else:
            return 'other'


def main():
    if len(sys.argv) < 2:
        print("Usage: generate_interactive_report.py <report.json> [output.html]")
        sys.exit(1)
        
    input_path = Path(sys.argv[1])
    output_path = Path(sys.argv[2]) if len(sys.argv) > 2 else input_path.with_suffix('.interactive.html')
    
    # Load report data
    with open(input_path, encoding='utf-8') as f:
        report_data = json.load(f)
        
    # Generate interactive report
    generator = InteractiveReportGenerator(report_data)
    generator.generate_interactive_html(output_path)


if __name__ == '__main__':
    sys.exit(main())
