#!/bin/bash

# Extract typedef to using conversions from the patch
# This script carefully extracts only the typedef changes

input_file="patch/extracted/source_changes.patch"
output_file="patch/extracted/patch_01_typedef_to_using.patch"

# Initialize output file
> "$output_file"

# Track current file being processed
current_file=""
current_hunk=""
in_hunk=false
hunk_has_typedef=false

while IFS= read -r line; do
    # Check for new file diff
    if [[ "$line" =~ ^diff\ --git ]]; then
        # If we were processing a hunk with typedef changes, write it
        if [ "$hunk_has_typedef" = true ]; then
            echo "$current_hunk" >> "$output_file"
        fi
        
        # Reset for new file
        current_file="$line"
        current_hunk=""
        in_hunk=false
        hunk_has_typedef=false
        
    # Check for hunk header
    elif [[ "$line" =~ ^@@ ]]; then
        # If previous hunk had typedef changes, write it
        if [ "$hunk_has_typedef" = true ]; then
            echo "$current_hunk" >> "$output_file"
        fi
        
        # Start new hunk
        if [ -n "$current_file" ]; then
            current_hunk="$current_file"$'\n'
            # Add file headers if this is the first hunk for this file
            if ! grep -q "$current_file" "$output_file" 2>/dev/null; then
                # Get the index and mode lines
                while IFS= read -r next_line && [[ ! "$next_line" =~ ^@@ ]]; do
                    current_hunk+="$next_line"$'\n'
                done < <(tail -n +$((line_num + 1)) "$input_file")
            fi
            current_file=""
        fi
        current_hunk+="$line"$'\n'
        in_hunk=true
        hunk_has_typedef=false
        
    # Check for typedef changes in the hunk
    elif [ "$in_hunk" = true ]; then
        current_hunk+="$line"$'\n'
        
        # Look for typedef removal or using addition
        if [[ "$line" =~ ^-.*typedef ]] || [[ "$line" =~ ^\+.*using.*= ]]; then
            hunk_has_typedef=true
        fi
        
        # End of hunk (empty line or new diff)
        if [ -z "$line" ] && [ "$hunk_has_typedef" = true ]; then
            echo "$current_hunk" >> "$output_file"
            in_hunk=false
            hunk_has_typedef=false
            current_hunk=""
        fi
    else
        if [ "$in_hunk" = true ]; then
            current_hunk+="$line"$'\n'
        fi
    fi
    
done < "$input_file"

# Write any remaining hunk
if [ "$hunk_has_typedef" = true ]; then
    echo "$current_hunk" >> "$output_file"
fi

echo "Extraction complete. Output written to $output_file"