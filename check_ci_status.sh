#!/bin/bash
# Quick script to check CI status for LPZRobots

echo "=== LPZRobots CI Status Check ==="
echo "Repository: jerryzhao173985/lpz"
echo "Time: $(date)"
echo

# Get latest workflow runs
runs=$(curl -s "https://api.github.com/repos/jerryzhao173985/lpz/actions/runs?per_page=5")

# Parse and display status
echo "$runs" | jq -r '.workflow_runs[] | 
  "\(.name) - Run #\(.run_number): \(
    if .status == "completed" then
      if .conclusion == "success" then "✅ PASSED"
      elif .conclusion == "failure" then "❌ FAILED"
      else "⚠️ " + .conclusion
      end
    else "🔄 " + .status
    end
  ) (started: \(.created_at | sub("T"; " ") | sub("Z"; "")))"' | head -10

echo
echo "=== Latest Run Details ==="
latest=$(echo "$runs" | jq -r '.workflow_runs[0]')
echo "$latest" | jq -r '"Workflow: \(.name)\nCommit: \(.head_commit.message | split("\n")[0])\nBranch: \(.head_branch)\nStatus: \(.status)\nConclusion: \(.conclusion // "pending")"'

# Check individual jobs if Simple CI
if [[ $(echo "$latest" | jq -r '.name') == "Simple CI" ]]; then
  run_id=$(echo "$latest" | jq -r '.id')
  echo
  echo "=== Simple CI Job Status ==="
  curl -s "https://api.github.com/repos/jerryzhao173985/lpz/actions/runs/$run_id/jobs" | \
    jq -r '.jobs[] | "\(.name): \(if .conclusion == "success" then "✅" elif .conclusion == "failure" then "❌" else "🔄" end) \(.conclusion // .status)"'
fi