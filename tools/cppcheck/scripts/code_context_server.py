#!/usr/bin/env python3
"""
Code Context Server for Cppcheck Reports
Provides code snippets and git information for issues
"""

import json
import subprocess
from pathlib import Path
from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib.parse

class CodeContextHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        """Handle GET requests for code context"""
        if self.path.startswith('/code'):
            self.handle_code_request()
        elif self.path.startswith('/git'):
            self.handle_git_request()
        else:
            self.send_error(404)
            
    def handle_code_request(self):
        """Return code context for a specific file and line"""
        params = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query)
        
        filepath = params.get('file', [''])[0]
        line = int(params.get('line', [1])[0])
        context = int(params.get('context', [5])[0])
        
        try:
            with open(filepath, 'r') as f:
                lines = f.readlines()
                
            start = max(0, line - context - 1)
            end = min(len(lines), line + context)
            
            result = {
                'file': filepath,
                'line': line,
                'context': []
            }
            
            for i in range(start, end):
                result['context'].append({
                    'number': i + 1,
                    'text': lines[i].rstrip('\n'),
                    'highlight': i + 1 == line
                })
                
            # Get git blame info
            try:
                blame = subprocess.run(
                    ['git', 'blame', '-L', f'{line},{line}', '--porcelain', filepath],
                    capture_output=True,
                    text=True
                )
                if blame.returncode == 0:
                    blame_lines = blame.stdout.split('\n')
                    commit = blame_lines[0].split()[0]
                    author = next((l.split(' ', 1)[1] for l in blame_lines if l.startswith('author ')), '')
                    date = next((l.split(' ', 1)[1] for l in blame_lines if l.startswith('author-time ')), '')
                    
                    result['blame'] = {
                        'commit': commit,
                        'author': author,
                        'date': date
                    }
            except (subprocess.CalledProcessError, ValueError) as e:
                print(f"Git blame error: {e}")
                pass
                
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(result).encode())
            
        except Exception as e:
            self.send_error(500, str(e))
            
    def handle_git_request(self):
        """Return git diff for a file"""
        params = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query)
        filepath = params.get('file', [''])[0]
        
        try:
            diff = subprocess.run(
                ['git', 'diff', 'HEAD', filepath],
                capture_output=True,
                text=True
            )
            
            result = {
                'file': filepath,
                'diff': diff.stdout
            }
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(result).encode())
            
        except Exception as e:
            self.send_error(500, str(e))
            
    def log_message(self, format, *args):
        """Suppress log messages"""
        pass


def start_server(port=8888):
    """Start the code context server"""
    server = HTTPServer(('localhost', port), CodeContextHandler)
    print(f"Code context server running on http://localhost:{port}")
    server.serve_forever()


if __name__ == '__main__':
    start_server()
