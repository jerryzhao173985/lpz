#!/usr/bin/env python3
"""Debug version of dashboard generator to diagnose issues"""

import json
import base64
from pathlib import Path

def generate_debug_dashboard(results, output_path):
    """Generate a debug dashboard with console logging"""
    
    issues = results.get('issues', [])
    issues_json = json.dumps(issues)
    issues_b64 = base64.b64encode(issues_json.encode()).decode()
    
    html = f"""<!DOCTYPE html>
<html>
<head>
    <title>Debug Dashboard</title>
    <style>
        body {{ font-family: Arial; padding: 20px; }}
        .issue {{ border: 1px solid #ccc; padding: 10px; margin: 5px 0; }}
        .error {{ background: #fee; }}
        .warning {{ background: #ffeaa7; }}
        .style {{ background: #e0e7ff; }}
        .performance {{ background: #d1fae5; }}
        #debug {{ background: #f0f0f0; padding: 20px; margin: 20px 0; }}
    </style>
</head>
<body>
    <h1>Debug Dashboard</h1>
    
    <div id="debug">
        <h3>Debug Info</h3>
        <div id="debugInfo"></div>
    </div>
    
    <h2>Issues</h2>
    <div id="issueList"></div>
    
    <script>
        console.log('=== DEBUG DASHBOARD START ===');
        
        // Try to decode issues
        let issuesData = [];
        let decodeError = null;
        
        try {{
            console.log('Attempting to decode base64 data...');
            const b64Data = '{issues_b64}';
            console.log('Base64 length:', b64Data.length);
            console.log('First 100 chars:', b64Data.substring(0, 100));
            
            const decoded = atob(b64Data);
            console.log('Decoded string length:', decoded.length);
            console.log('First 100 chars of decoded:', decoded.substring(0, 100));
            
            issuesData = JSON.parse(decoded);
            console.log('Successfully parsed', issuesData.length, 'issues');
        }} catch (e) {{
            decodeError = e;
            console.error('Failed to decode issues:', e);
        }}
        
        // Display debug info
        const debugDiv = document.getElementById('debugInfo');
        debugDiv.innerHTML = `
            <p>Issues found: ${{issuesData.length}}</p>
            <p>Decode error: ${{decodeError ? decodeError.toString() : 'None'}}</p>
            <p>First issue: ${{issuesData.length > 0 ? JSON.stringify(issuesData[0]).substring(0, 200) + '...' : 'None'}}</p>
        `;
        
        // Display issues
        const listDiv = document.getElementById('issueList');
        
        if (issuesData.length === 0) {{
            listDiv.innerHTML = '<p style="color: red;">No issues to display!</p>';
        }} else {{
            issuesData.forEach((issue, index) => {{
                const div = document.createElement('div');
                div.className = 'issue ' + (issue.severity || 'unknown');
                div.innerHTML = `
                    <strong>#${{index + 1}} ${{issue.file}}:${{issue.line}}</strong><br>
                    <span style="font-weight: bold;">[${{issue.severity || 'unknown'}}]</span> 
                    ${{issue.message || 'No message'}}<br>
                    <em>ID: ${{issue.id || 'No ID'}}</em>
                `;
                listDiv.appendChild(div);
            }});
        }}
        
        console.log('=== DEBUG DASHBOARD END ===');
    </script>
</body>
</html>"""
    
    with open(output_path, 'w') as f:
        f.write(html)
    print(f"Debug dashboard written to: {output_path}")

# Test with the actual results
if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        json_path = sys.argv[1]
        with open(json_path) as f:
            results = json.load(f)
        generate_debug_dashboard(results, '/tmp/debug_dashboard.html')
    else:
        print("Usage: python generate_debug_dashboard.py <report.json>")