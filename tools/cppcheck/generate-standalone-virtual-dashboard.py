#!/usr/bin/env python3
"""
Standalone Virtual Scroll Dashboard Generator
Embeds JSONL data as JavaScript to avoid CORS issues
"""

import json
from pathlib import Path
from datetime import datetime
import hashlib

class StandaloneVirtualDashboardGenerator:
    def __init__(self, issues_file):
        with open(issues_file) as f:
            data = json.load(f)
        self.issues = data.get('issues', [])
        self.timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        
        # Generate unique IDs for each issue
        for i, issue in enumerate(self.issues):
            if 'id' not in issue:
                id_str = f"{issue.get('file', '')}:{issue.get('line', '')}:{issue.get('message', '')}"
                issue['id'] = hashlib.md5(id_str.encode()).hexdigest()[:8].upper()
    
    def generate(self, output_file):
        """Generate standalone dashboard with embedded data"""
        
        # Calculate statistics
        stats = self.calculate_stats()
        
        # Prepare data
        issues_without_context = []
        code_context_map = {}
        
        for issue in self.issues:
            # Create issue without code context
            issue_copy = {k: v for k, v in issue.items() if k != 'code_context'}
            issues_without_context.append(issue_copy)
            
            # Store code context separately
            if 'code_context' in issue and issue.get('id'):
                code_context_map[issue['id']] = issue['code_context']
        
        # Count issues with code context
        with_context = len(code_context_map)
        
        # Generate JSONL strings
        issues_jsonl = '\n'.join(json.dumps(issue) for issue in issues_without_context)
        code_jsonl_entries = []
        for issue_id, context in code_context_map.items():
            code_jsonl_entries.append(json.dumps({'id': issue_id, 'code_context': context}))
        code_jsonl = '\n'.join(code_jsonl_entries)
        
        # Generate HTML
        html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CPPCheck Studio - Standalone Virtual Dashboard</title>
    
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css" rel="stylesheet">
    
    <style>
        {self.generate_styles()}
    </style>
</head>
<body>
    <div class="container">
        <!-- Header -->
        <header class="header">
            <div class="header-content">
                <h1><i class="fas fa-code"></i> CPPCheck Studio - Virtual Scroll Dashboard</h1>
                <div class="header-info">
                    <span><i class="fas fa-project-diagram"></i> LPZRobots</span>
                    <span><i class="fas fa-clock"></i> {self.timestamp}</span>
                    <span><i class="fas fa-database"></i> {len(self.issues)} total issues</span>
                    <span><i class="fas fa-file-code"></i> {with_context} with code context</span>
                </div>
            </div>
        </header>
        
        <!-- Statistics Cards -->
        <div class="stats-grid">
            <div class="stat-card error">
                <i class="fas fa-exclamation-circle"></i>
                <h3>Errors</h3>
                <div class="value">{stats['errors']}</div>
                <div class="percent">{stats['error_percent']:.1f}%</div>
            </div>
            
            <div class="stat-card warning">
                <i class="fas fa-exclamation-triangle"></i>
                <h3>Warnings</h3>
                <div class="value">{stats['warnings']}</div>
                <div class="percent">{stats['warning_percent']:.1f}%</div>
            </div>
            
            <div class="stat-card style">
                <i class="fas fa-palette"></i>
                <h3>Style</h3>
                <div class="value">{stats['style']}</div>
                <div class="percent">{stats['style_percent']:.1f}%</div>
            </div>
            
            <div class="stat-card performance">
                <i class="fas fa-tachometer-alt"></i>
                <h3>Performance</h3>
                <div class="value">{stats['performance']}</div>
                <div class="percent">{stats['performance_percent']:.1f}%</div>
            </div>
        </div>
        
        <!-- Filter Controls -->
        <div class="controls">
            <div class="search-container">
                <i class="fas fa-search"></i>
                <input type="text" id="searchInput" placeholder="Search by file, message, or ID..." onkeyup="debounce(filterData, 300)()">
            </div>
            
            <div class="filter-buttons">
                <button class="filter-btn active" onclick="setSeverityFilter('all', this)">
                    <i class="fas fa-list"></i> All ({stats['total']})
                </button>
                <button class="filter-btn" onclick="setSeverityFilter('error', this)">
                    <i class="fas fa-exclamation-circle"></i> Errors ({stats['errors']})
                </button>
                <button class="filter-btn" onclick="setSeverityFilter('warning', this)">
                    <i class="fas fa-exclamation-triangle"></i> Warnings ({stats['warnings']})
                </button>
                <button class="filter-btn" onclick="setSeverityFilter('style', this)">
                    <i class="fas fa-palette"></i> Style ({stats['style']})
                </button>
                <button class="filter-btn" onclick="setSeverityFilter('performance', this)">
                    <i class="fas fa-tachometer-alt"></i> Performance ({stats['performance']})
                </button>
                <button class="filter-btn" onclick="setSeverityFilter('information', this)">
                    <i class="fas fa-info-circle"></i> Info
                </button>
            </div>
        </div>
        
        <!-- Issues Count and Loading Status -->
        <div class="status-bar">
            <div class="issues-count">
                <span id="issuesCount">Loading...</span>
            </div>
            <div class="loading-status" id="loadingStatus" style="display: none;">
                <i class="fas fa-spinner fa-spin"></i> <span id="loadingText">Loading...</span>
            </div>
        </div>
        
        <!-- Virtual Scroll Container -->
        <div class="virtual-scroll-container" id="scrollContainer">
            <div class="issues-table-wrapper">
                <table class="issues-table">
                    <thead>
                        <tr>
                            <th class="col-indicator"></th>
                            <th class="col-file">FILE</th>
                            <th class="col-line">LINE</th>
                            <th class="col-severity">SEVERITY</th>
                            <th class="col-message">MESSAGE</th>
                            <th class="col-id">ID</th>
                            <th class="col-actions">ACTIONS</th>
                        </tr>
                    </thead>
                </table>
                <div class="virtual-scroll-viewport" id="viewport">
                    <div class="virtual-scroll-spacer" id="spacerTop"></div>
                    <table class="issues-table">
                        <tbody id="issuesBody">
                            <!-- Virtual rows will be rendered here -->
                        </tbody>
                    </table>
                    <div class="virtual-scroll-spacer" id="spacerBottom"></div>
                </div>
            </div>
        </div>
        
        <!-- Code Preview Modal -->
        <div id="codeModal" class="modal">
            <div class="modal-content">
                <div class="modal-header">
                    <h3 id="modalTitle">Issue Details</h3>
                    <button onclick="closeModal()" class="close-btn">&times;</button>
                </div>
                <div class="modal-body" id="modalBody">
                    <!-- Content will be inserted here -->
                </div>
            </div>
        </div>
    </div>
    
    <!-- Embedded JSONL Data -->
    <script id="issuesData" type="application/x-ndjson">
{issues_jsonl}
    </script>
    
    <script id="codeContextData" type="application/x-ndjson">
{code_jsonl}
    </script>
    
    <script>
        // Configuration
        const CONFIG = {{
            ROW_HEIGHT: 50,
            VISIBLE_BUFFER: 5,
            SCROLL_DEBOUNCE: 10,
            SEARCH_DEBOUNCE: 300,
            BATCH_SIZE: 50
        }};
        
        // Global state
        const state = {{
            allIssues: [],
            filteredIssues: [],
            codeContextMap: new Map(),
            currentFilter: 'all',
            currentSearch: '',
            visibleStart: 0,
            visibleEnd: 0,
            isLoading: false,
            scrollTop: 0,
            containerHeight: 0
        }};
        
        // Initialize
        function initialize() {{
            try {{
                showLoadingStatus('Loading issues data...');
                
                // Load issues from embedded JSONL
                loadEmbeddedData();
                
                // Set up virtual scrolling
                setupVirtualScroll();
                
                // Initial render
                filterData();
                
                hideLoadingStatus();
            }} catch (error) {{
                console.error('Initialization error:', error);
                alert('Failed to load dashboard: ' + error.message);
            }}
        }}
        
        // Load embedded JSONL data
        function loadEmbeddedData() {{
            try {{
                // Parse issues data
                const issuesScript = document.getElementById('issuesData');
                const issuesText = issuesScript.textContent.trim();
                const issuesLines = issuesText.split('\\n').filter(line => line.trim());
                
                state.allIssues = issuesLines.map(line => {{
                    try {{
                        return JSON.parse(line);
                    }} catch (e) {{
                        console.error('Failed to parse issue line:', e);
                        return null;
                    }}
                }}).filter(Boolean);
                
                console.log('Loaded', state.allIssues.length, 'issues');
                
                // Parse code context data
                const codeScript = document.getElementById('codeContextData');
                const codeText = codeScript.textContent.trim();
                const codeLines = codeText.split('\\n').filter(line => line.trim());
                
                codeLines.forEach(line => {{
                    try {{
                        const data = JSON.parse(line);
                        if (data.id && data.code_context) {{
                            state.codeContextMap.set(data.id, data.code_context);
                        }}
                    }} catch (e) {{
                        console.error('Failed to parse code context:', e);
                    }}
                }});
                
                console.log('Loaded code context for', state.codeContextMap.size, 'issues');
                
            }} catch (error) {{
                console.error('Failed to load embedded data:', error);
                throw error;
            }}
        }}
        
        // Set up virtual scrolling
        function setupVirtualScroll() {{
            const viewport = document.getElementById('viewport');
            const scrollContainer = document.getElementById('scrollContainer');
            
            // Update container height on resize
            const updateContainerHeight = () => {{
                state.containerHeight = scrollContainer.clientHeight - 100; // Account for header
                renderVisibleRows();
            }};
            
            updateContainerHeight();
            window.addEventListener('resize', updateContainerHeight);
            
            // Handle scroll events
            scrollContainer.addEventListener('scroll', debounce(() => {{
                state.scrollTop = scrollContainer.scrollTop;
                renderVisibleRows();
            }}, CONFIG.SCROLL_DEBOUNCE));
        }}
        
        // Render visible rows based on scroll position
        function renderVisibleRows() {{
            const totalHeight = state.filteredIssues.length * CONFIG.ROW_HEIGHT;
            const visibleStart = Math.floor(state.scrollTop / CONFIG.ROW_HEIGHT) - CONFIG.VISIBLE_BUFFER;
            const visibleEnd = Math.ceil((state.scrollTop + state.containerHeight) / CONFIG.ROW_HEIGHT) + CONFIG.VISIBLE_BUFFER;
            
            state.visibleStart = Math.max(0, visibleStart);
            state.visibleEnd = Math.min(state.filteredIssues.length, visibleEnd);
            
            // Update spacers
            document.getElementById('spacerTop').style.height = (state.visibleStart * CONFIG.ROW_HEIGHT) + 'px';
            document.getElementById('spacerBottom').style.height = 
                ((state.filteredIssues.length - state.visibleEnd) * CONFIG.ROW_HEIGHT) + 'px';
            
            // Get visible issues
            const visibleIssues = state.filteredIssues.slice(state.visibleStart, state.visibleEnd);
            
            // Render rows
            const tbody = document.getElementById('issuesBody');
            tbody.innerHTML = '';
            
            visibleIssues.forEach((issue, index) => {{
                const row = createIssueRow(issue, state.visibleStart + index);
                tbody.appendChild(row);
            }});
        }}
        
        // Create issue row
        function createIssueRow(issue, globalIndex) {{
            const row = document.createElement('tr');
            row.className = 'issue-row';
            row.dataset.id = issue.id;
            
            const hasCodeContext = state.codeContextMap.has(issue.id);
            
            // Indicator cell (for code context)
            const indicatorCell = document.createElement('td');
            indicatorCell.className = 'indicator-cell';
            if (hasCodeContext) {{
                indicatorCell.innerHTML = '<div class="code-indicator"></div>';
            }}
            
            // File cell
            const fileCell = document.createElement('td');
            fileCell.className = 'file-cell';
            fileCell.title = issue.file || '';
            fileCell.innerHTML = '<i class="fas fa-file-code"></i> ' + escapeHtml(getFileName(issue.file || ''));
            
            // Line cell
            const lineCell = document.createElement('td');
            lineCell.className = 'line-cell';
            lineCell.textContent = issue.line || '-';
            
            // Severity cell
            const severityCell = document.createElement('td');
            const severityBadge = document.createElement('span');
            severityBadge.className = 'severity-badge ' + (issue.severity || 'unknown');
            severityBadge.textContent = (issue.severity || 'UNKNOWN').toUpperCase();
            severityCell.appendChild(severityBadge);
            
            // Message cell
            const messageCell = document.createElement('td');
            messageCell.className = 'message-cell';
            messageCell.title = issue.message || '';
            messageCell.textContent = truncateMessage(issue.message || 'No message');
            
            // ID cell
            const idCell = document.createElement('td');
            idCell.className = 'id-cell';
            idCell.textContent = issue.id || 'N/A';
            
            // Actions cell
            const actionsCell = document.createElement('td');
            actionsCell.className = 'actions-cell';
            const actionBtn = document.createElement('button');
            actionBtn.className = 'action-btn' + (hasCodeContext ? ' has-code' : '');
            actionBtn.innerHTML = '<i class="fas ' + (hasCodeContext ? 'fa-code' : 'fa-eye') + '"></i>';
            actionBtn.onclick = (e) => {{
                e.stopPropagation();
                showIssueDetails(issue, globalIndex);
            }};
            actionsCell.appendChild(actionBtn);
            
            // Add cells to row
            row.appendChild(indicatorCell);
            row.appendChild(fileCell);
            row.appendChild(lineCell);
            row.appendChild(severityCell);
            row.appendChild(messageCell);
            row.appendChild(idCell);
            row.appendChild(actionsCell);
            
            // Row click handler
            row.onclick = () => showIssueDetails(issue, globalIndex);
            
            return row;
        }}
        
        // Filter data based on search and severity
        function filterData() {{
            const searchTerm = document.getElementById('searchInput').value.toLowerCase();
            state.currentSearch = searchTerm;
            
            state.filteredIssues = state.allIssues.filter(issue => {{
                // Severity filter
                if (state.currentFilter !== 'all' && issue.severity !== state.currentFilter) {{
                    return false;
                }}
                
                // Search filter
                if (searchTerm) {{
                    const matchFile = (issue.file || '').toLowerCase().includes(searchTerm);
                    const matchMessage = (issue.message || '').toLowerCase().includes(searchTerm);
                    const matchId = (issue.id || '').toLowerCase().includes(searchTerm);
                    return matchFile || matchMessage || matchId;
                }}
                
                return true;
            }});
            
            // Update count
            updateIssueCount();
            
            // Reset scroll and render
            document.getElementById('scrollContainer').scrollTop = 0;
            state.scrollTop = 0;
            renderVisibleRows();
        }}
        
        // Update issue count display
        function updateIssueCount() {{
            const countEl = document.getElementById('issuesCount');
            const filtered = state.filteredIssues.length;
            const total = state.allIssues.length;
            
            if (filtered === total) {{
                countEl.textContent = `Showing all ${{total}} issues`;
            }} else {{
                countEl.textContent = `Showing ${{filtered}} of ${{total}} issues`;
            }}
        }}
        
        // Set severity filter
        function setSeverityFilter(severity, button) {{
            state.currentFilter = severity;
            
            // Update button states
            document.querySelectorAll('.filter-btn').forEach(btn => {{
                btn.classList.remove('active');
            }});
            button.classList.add('active');
            
            filterData();
        }}
        
        // Show issue details modal
        function showIssueDetails(issue, index) {{
            const modal = document.getElementById('codeModal');
            const modalTitle = document.getElementById('modalTitle');
            const modalBody = document.getElementById('modalBody');
            
            modalTitle.innerHTML = '<i class="fas fa-file-code"></i> ' + 
                escapeHtml(getFileName(issue.file || 'Unknown')) + ':' + (issue.line || '?');
            
            const codeContext = state.codeContextMap.get(issue.id);
            
            // Build modal content
            let content = '<div class="issue-details">';
            
            // Issue info
            content += '<div class="info-section">';
            content += '<h4><i class="fas fa-info-circle"></i> Issue Information</h4>';
            content += '<table class="info-table">';
            content += '<tr><td><strong>File:</strong></td><td class="code-text">' + escapeHtml(issue.file || 'Unknown') + '</td></tr>';
            content += '<tr><td><strong>Line:</strong></td><td>' + (issue.line || 'N/A') + '</td></tr>';
            content += '<tr><td><strong>Severity:</strong></td><td><span class="severity-badge ' + (issue.severity || 'unknown') + '">' + (issue.severity || 'UNKNOWN').toUpperCase() + '</span></td></tr>';
            content += '<tr><td><strong>Issue ID:</strong></td><td><code>' + (issue.id || 'N/A') + '</code></td></tr>';
            content += '<tr><td><strong>Position:</strong></td><td>' + (index + 1) + ' of ' + state.filteredIssues.length + '</td></tr>';
            content += '</table></div>';
            
            // Message
            content += '<div class="message-section">';
            content += '<h4><i class="fas fa-comment-alt"></i> Message</h4>';
            content += '<div class="message-box">' + escapeHtml(issue.message || 'No message available') + '</div>';
            content += '</div>';
            
            // Code context
            content += '<div class="code-section">';
            content += '<h4><i class="fas fa-code"></i> Code Context</h4>';
            
            if (codeContext && codeContext.lines && codeContext.lines.length > 0) {{
                content += '<div class="code-preview"><pre><code class="language-cpp">';
                
                codeContext.lines.forEach(line => {{
                    const isTarget = line.is_target === true;
                    const lineNum = String(line.number || 0).padStart(4, ' ');
                    const lineContent = escapeHtml(line.content || '');
                    
                    if (isTarget) {{
                        content += '<span class="highlight-line">' + lineNum + ': ' + lineContent + '</span>\\n';
                    }} else {{
                        content += lineNum + ': ' + lineContent + '\\n';
                    }}
                }});
                
                content += '</code></pre></div>';
            }} else {{
                content += '<div class="no-code-message">';
                content += '<i class="fas fa-info-circle"></i>';
                content += '<p>Code context not available for this issue.</p>';
                content += '</div>';
            }}
            
            content += '</div></div>';
            
            modalBody.innerHTML = content;
            modal.style.display = 'block';
        }}
        
        // Helper functions
        function escapeHtml(text) {{
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }}
        
        function getFileName(path) {{
            if (!path) return 'Unknown';
            const parts = path.split('/');
            return parts[parts.length - 1];
        }}
        
        function truncateMessage(message) {{
            const maxLength = 80;
            if (message && message.length > maxLength) {{
                return message.substring(0, maxLength - 3) + '...';
            }}
            return message || '';
        }}
        
        function debounce(func, wait) {{
            let timeout;
            return function executedFunction(...args) {{
                const later = () => {{
                    clearTimeout(timeout);
                    func(...args);
                }};
                clearTimeout(timeout);
                timeout = setTimeout(later, wait);
            }};
        }}
        
        function showLoadingStatus(text) {{
            document.getElementById('loadingStatus').style.display = 'block';
            document.getElementById('loadingText').textContent = text;
        }}
        
        function hideLoadingStatus() {{
            document.getElementById('loadingStatus').style.display = 'none';
        }}
        
        function closeModal() {{
            document.getElementById('codeModal').style.display = 'none';
        }}
        
        // Close modal on outside click
        window.onclick = function(event) {{
            const modal = document.getElementById('codeModal');
            if (event.target === modal) {{
                closeModal();
            }}
        }};
        
        // Initialize when DOM is ready
        if (document.readyState === 'loading') {{
            document.addEventListener('DOMContentLoaded', initialize);
        }} else {{
            initialize();
        }}
    </script>
</body>
</html>"""
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(html_content)
            
        print(f"✅ Standalone virtual scroll dashboard generated: {output_file}")
        print(f"   Total issues: {len(self.issues)}")
        print(f"   Issues with code context: {with_context}")
        print(f"   File size: {Path(output_file).stat().st_size / 1024 / 1024:.1f} MB")
        print(f"   No server required - works with file:// protocol")
        
    def calculate_stats(self):
        """Calculate issue statistics"""
        total = len(self.issues)
        if total == 0:
            return {
                'total': 0,
                'errors': 0,
                'warnings': 0,
                'style': 0,
                'performance': 0,
                'information': 0,
                'error_percent': 0,
                'warning_percent': 0,
                'style_percent': 0,
                'performance_percent': 0
            }
            
        stats = {
            'total': total,
            'errors': sum(1 for i in self.issues if i.get('severity') == 'error'),
            'warnings': sum(1 for i in self.issues if i.get('severity') == 'warning'),
            'style': sum(1 for i in self.issues if i.get('severity') == 'style'),
            'performance': sum(1 for i in self.issues if i.get('severity') == 'performance'),
            'information': sum(1 for i in self.issues if i.get('severity') == 'information')
        }
        
        # Calculate percentages
        stats['error_percent'] = (stats['errors'] / total) * 100 if total > 0 else 0
        stats['warning_percent'] = (stats['warnings'] / total) * 100 if total > 0 else 0
        stats['style_percent'] = (stats['style'] / total) * 100 if total > 0 else 0
        stats['performance_percent'] = (stats['performance'] / total) * 100 if total > 0 else 0
            
        return stats
        
    def generate_styles(self):
        """Generate CSS styles with fixed alignment"""
        return """
        * { box-sizing: border-box; margin: 0; padding: 0; }
        
        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
            background: #f5f7fa;
            color: #2d3748;
            line-height: 1.6;
            overflow: hidden;
        }
        
        .container {
            height: 100vh;
            display: flex;
            flex-direction: column;
        }
        
        /* Header */
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px 0;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            flex-shrink: 0;
        }
        
        .header-content {
            max-width: 1600px;
            margin: 0 auto;
            padding: 0 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            flex-wrap: wrap;
            gap: 20px;
        }
        
        .header h1 {
            font-size: 1.8em;
            font-weight: 700;
        }
        
        .header-info {
            display: flex;
            gap: 20px;
            font-size: 0.9em;
            opacity: 0.9;
        }
        
        /* Statistics Grid */
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            padding: 20px;
            max-width: 1600px;
            margin: 0 auto;
            width: 100%;
            flex-shrink: 0;
        }
        
        .stat-card {
            background: white;
            border-radius: 8px;
            padding: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.05);
            text-align: center;
            transition: transform 0.2s, box-shadow 0.2s;
            position: relative;
            overflow: hidden;
        }
        
        .stat-card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            height: 3px;
            background: currentColor;
        }
        
        .stat-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(0,0,0,0.1);
        }
        
        .stat-card i {
            font-size: 2em;
            margin-bottom: 8px;
            opacity: 0.8;
        }
        
        .stat-card h3 {
            font-size: 0.85em;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            color: #718096;
            margin-bottom: 8px;
        }
        
        .stat-card .value {
            font-size: 2em;
            font-weight: 700;
            margin-bottom: 4px;
        }
        
        .stat-card .percent {
            font-size: 0.85em;
            color: #718096;
        }
        
        /* Card colors */
        .stat-card.error { color: #e53e3e; }
        .stat-card.warning { color: #dd6b20; }
        .stat-card.style { color: #5a67d8; }
        .stat-card.performance { color: #38a169; }
        
        /* Controls */
        .controls {
            background: white;
            border-radius: 8px;
            padding: 15px;
            margin: 0 20px;
            max-width: 1560px;
            align-self: center;
            width: calc(100% - 40px);
            box-shadow: 0 2px 4px rgba(0,0,0,0.05);
            display: flex;
            gap: 20px;
            flex-wrap: wrap;
            align-items: center;
            flex-shrink: 0;
        }
        
        .search-container {
            flex: 1;
            min-width: 300px;
            position: relative;
        }
        
        .search-container i {
            position: absolute;
            left: 15px;
            top: 50%;
            transform: translateY(-50%);
            color: #a0aec0;
        }
        
        .search-container input {
            width: 100%;
            padding: 10px 15px 10px 40px;
            border: 1px solid #e2e8f0;
            border-radius: 6px;
            font-size: 0.95em;
            transition: border-color 0.2s;
        }
        
        .search-container input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        
        .filter-buttons {
            display: flex;
            gap: 8px;
            flex-wrap: wrap;
        }
        
        .filter-btn {
            padding: 8px 14px;
            border: 1px solid #e2e8f0;
            background: white;
            border-radius: 6px;
            cursor: pointer;
            font-size: 0.85em;
            transition: all 0.2s;
            display: flex;
            align-items: center;
            gap: 5px;
            font-family: inherit;
        }
        
        .filter-btn:hover {
            background: #f7fafc;
            transform: translateY(-1px);
        }
        
        .filter-btn.active {
            background: #667eea;
            color: white;
            border-color: #667eea;
        }
        
        /* Status bar */
        .status-bar {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 20px;
            max-width: 1600px;
            margin: 0 auto;
            width: 100%;
            flex-shrink: 0;
        }
        
        .issues-count {
            font-size: 0.85em;
            color: #718096;
        }
        
        .loading-status {
            font-size: 0.85em;
            color: #667eea;
        }
        
        /* Virtual scroll container */
        .virtual-scroll-container {
            flex: 1;
            overflow-y: auto;
            background: white;
            margin: 0 20px 20px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.05);
            max-width: 1560px;
            align-self: center;
            width: calc(100% - 40px);
        }
        
        .issues-table-wrapper {
            position: relative;
        }
        
        /* Issues Table */
        .issues-table {
            width: 100%;
            border-collapse: collapse;
            table-layout: fixed;
        }
        
        .issues-table thead {
            background: #f7fafc;
            border-bottom: 2px solid #e2e8f0;
            position: sticky;
            top: 0;
            z-index: 10;
        }
        
        .issues-table th {
            padding: 12px 15px;
            text-align: left;
            font-weight: 600;
            font-size: 0.85em;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            color: #4a5568;
            white-space: nowrap;
        }
        
        /* Column widths - Fixed to prevent shifting */
        .col-indicator { width: 20px; padding: 0 5px; }
        .col-file { width: 25%; }
        .col-line { width: 80px; text-align: center; }
        .col-severity { width: 120px; }
        .col-message { width: calc(100% - 25% - 80px - 120px - 100px - 80px - 20px); }
        .col-id { width: 100px; }
        .col-actions { width: 80px; text-align: center; }
        
        /* Virtual scroll viewport */
        .virtual-scroll-viewport {
            position: relative;
        }
        
        .virtual-scroll-spacer {
            width: 100%;
        }
        
        /* Issue rows */
        .issue-row {
            height: 50px;
            border-bottom: 1px solid #e2e8f0;
            transition: background 0.1s;
            cursor: pointer;
        }
        
        .issue-row:hover {
            background: #f7fafc;
        }
        
        .issue-row td {
            padding: 0 15px;
            font-size: 0.95em;
            height: 50px;
            vertical-align: middle;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }
        
        /* Code indicator - Now in separate column */
        .indicator-cell {
            width: 20px;
            padding: 0 5px !important;
        }
        
        .code-indicator {
            width: 4px;
            height: 30px;
            background: #667eea;
            border-radius: 2px;
        }
        
        .file-cell {
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
            color: #4a5568;
        }
        
        .line-cell {
            font-family: 'Monaco', 'Consolas', monospace;
            color: #718096;
            text-align: center;
        }
        
        .message-cell {
            color: #2d3748;
        }
        
        .id-cell {
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.85em;
            color: #718096;
        }
        
        .actions-cell {
            text-align: center;
        }
        
        .severity-badge {
            display: inline-block;
            padding: 4px 10px;
            border-radius: 4px;
            font-size: 0.75em;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }
        
        .severity-badge.error {
            background: #fed7d7;
            color: #c53030;
        }
        
        .severity-badge.warning {
            background: #feebc8;
            color: #c05621;
        }
        
        .severity-badge.style {
            background: #e0e7ff;
            color: #3730a3;
        }
        
        .severity-badge.performance {
            background: #d1fae5;
            color: #065f46;
        }
        
        .severity-badge.information {
            background: #e0f2fe;
            color: #0369a1;
        }
        
        .severity-badge.unknown {
            background: #e2e8f0;
            color: #4a5568;
        }
        
        .action-btn {
            padding: 6px 10px;
            border: 1px solid #e2e8f0;
            background: white;
            border-radius: 4px;
            cursor: pointer;
            font-size: 0.9em;
            transition: all 0.2s;
            color: #4a5568;
        }
        
        .action-btn:hover {
            background: #667eea;
            color: white;
            border-color: #667eea;
            transform: scale(1.05);
        }
        
        .action-btn.has-code {
            border-color: #667eea;
            color: #667eea;
        }
        
        /* Modal */
        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0,0,0,0.5);
            z-index: 1000;
            backdrop-filter: blur(4px);
        }
        
        .modal-content {
            background: white;
            margin: 50px auto;
            width: 90%;
            max-width: 1000px;
            max-height: 90vh;
            border-radius: 12px;
            overflow: hidden;
            box-shadow: 0 20px 40px rgba(0,0,0,0.2);
        }
        
        .modal-header {
            background: #f7fafc;
            padding: 20px;
            border-bottom: 1px solid #e2e8f0;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        
        .modal-header h3 {
            font-size: 1.2em;
            font-weight: 600;
            color: #2d3748;
        }
        
        .close-btn {
            background: none;
            border: none;
            font-size: 1.5em;
            cursor: pointer;
            color: #718096;
            padding: 0;
            width: 30px;
            height: 30px;
            display: flex;
            align-items: center;
            justify-content: center;
            border-radius: 4px;
            transition: all 0.2s;
        }
        
        .close-btn:hover {
            background: #e2e8f0;
            color: #2d3748;
        }
        
        .modal-body {
            padding: 0;
            max-height: calc(90vh - 80px);
            overflow-y: auto;
        }
        
        /* Issue Details */
        .issue-details {
            padding: 20px;
        }
        
        .info-section, .message-section, .code-section {
            margin-bottom: 25px;
        }
        
        .issue-details h4 {
            font-size: 1em;
            font-weight: 600;
            color: #2d3748;
            margin-bottom: 15px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .info-table {
            width: 100%;
            background: #f7fafc;
            border-radius: 8px;
            overflow: hidden;
        }
        
        .info-table td {
            padding: 10px 15px;
            border-bottom: 1px solid #e2e8f0;
        }
        
        .info-table tr:last-child td {
            border-bottom: none;
        }
        
        .info-table strong {
            color: #4a5568;
            font-weight: 500;
            display: inline-block;
            min-width: 100px;
        }
        
        .code-text {
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
        }
        
        .message-box {
            background: #f7fafc;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            padding: 15px;
            font-size: 0.95em;
            line-height: 1.6;
        }
        
        /* Code Preview */
        .code-preview {
            background: #1e1e1e;
            border-radius: 8px;
            overflow: hidden;
            position: relative;
        }
        
        .code-preview pre {
            margin: 0;
            padding: 20px;
            overflow-x: auto;
            background: #1e1e1e;
        }
        
        .code-preview code {
            font-family: 'Monaco', 'Consolas', monospace;
            font-size: 0.9em;
            line-height: 1.6;
            color: #d4d4d4;
            display: block;
        }
        
        .highlight-line {
            background: rgba(255, 235, 59, 0.1);
            border-left: 3px solid #ffd600;
            display: block;
            margin: 0 -20px;
            padding: 0 20px;
            padding-left: 17px;
        }
        
        .no-code-message {
            text-align: center;
            padding: 40px;
            color: #718096;
            background: #f7fafc;
            border-radius: 8px;
        }
        
        .no-code-message i {
            font-size: 3em;
            margin-bottom: 15px;
            opacity: 0.5;
        }
        
        .no-code-message p {
            margin: 5px 0;
        }
        
        /* Scrollbar styling */
        .virtual-scroll-container::-webkit-scrollbar {
            width: 10px;
        }
        
        .virtual-scroll-container::-webkit-scrollbar-track {
            background: #f1f1f1;
        }
        
        .virtual-scroll-container::-webkit-scrollbar-thumb {
            background: #888;
            border-radius: 5px;
        }
        
        .virtual-scroll-container::-webkit-scrollbar-thumb:hover {
            background: #555;
        }
        
        /* Responsive design */
        @media (max-width: 768px) {
            .header-content {
                flex-direction: column;
                text-align: center;
            }
            
            .header-info {
                flex-wrap: wrap;
                justify-content: center;
            }
            
            .stats-grid {
                grid-template-columns: 1fr 1fr;
            }
            
            .filter-buttons {
                justify-content: center;
            }
            
            .col-file { width: 30%; }
            .col-message { width: calc(100% - 30% - 60px - 100px - 80px - 60px - 20px); }
        }
        """

if __name__ == '__main__':
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: generate-standalone-virtual-dashboard.py <analysis.json> [output.html]")
        sys.exit(1)
        
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else 'standalone-virtual-dashboard.html'
    
    generator = StandaloneVirtualDashboardGenerator(input_file)
    generator.generate(output_file)