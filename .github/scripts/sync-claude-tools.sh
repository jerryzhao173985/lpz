#!/bin/bash
# Script to sync Claude allowed tools from single source to both workflow files
# This ensures consistency and avoids manual editing of multiple files

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
TOOLS_FILE="$PROJECT_ROOT/.github/claude-allowed-tools-lpzrobots-complete.txt"
CLAUDE_WORKFLOW="$PROJECT_ROOT/.github/workflows/claude.yml"
REVIEW_WORKFLOW="$PROJECT_ROOT/.github/workflows/claude-code-review.yml"

# Check if tools file exists
if [ ! -f "$TOOLS_FILE" ]; then
    echo "❌ Error: Tools file not found: $TOOLS_FILE"
    exit 1
fi

# Extract tools list (remove comments and empty lines, join into single line)
TOOLS=$(grep -v '^#' "$TOOLS_FILE" | grep -v '^$' | tr -d '\n' | sed 's/,$//')

# Count tools for verification
TOOL_COUNT=$(echo "$TOOLS" | tr ',' '\n' | grep -v '^$' | wc -l | tr -d ' ')

echo "📋 Found $TOOL_COUNT tools in $TOOLS_FILE"

# Function to update a workflow file
update_workflow() {
    local workflow_file="$1"
    local workflow_name="$(basename "$workflow_file")"
    
    if [ ! -f "$workflow_file" ]; then
        echo "❌ Error: Workflow file not found: $workflow_file"
        return 1
    fi
    
    echo "🔄 Updating $workflow_name..."
    
    # Create a temporary file
    local temp_file="${workflow_file}.tmp"
    
    # Process the file line by line
    local in_allowed_tools=false
    while IFS= read -r line; do
        if [[ "$line" =~ ^[[:space:]]*allowed_tools:[[:space:]]* ]]; then
            # Found allowed_tools line - replace it
            echo "          allowed_tools: \"$TOOLS\"" >> "$temp_file"
            in_allowed_tools=false
        else
            echo "$line" >> "$temp_file"
        fi
    done < "$workflow_file"
    
    # Replace the original file
    mv "$temp_file" "$workflow_file"
    echo "✅ Updated $workflow_name"
}

# Update both workflows
update_workflow "$CLAUDE_WORKFLOW"
update_workflow "$REVIEW_WORKFLOW"

echo ""
echo "🎉 Successfully synced tools to both workflows!"
echo ""
echo "📊 Summary:"
echo "   - Total tools: $TOOL_COUNT"
echo "   - Source: $(basename "$TOOLS_FILE")"
echo "   - Updated: claude.yml, claude-code-review.yml"
echo ""
echo "💡 To verify the changes:"
echo "   git diff .github/workflows/"