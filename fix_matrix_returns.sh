#!/bin/bash
# Fix Matrix return statements that need explicit constructor

echo "Fixing Matrix return statements..."

# Function to fix Matrix returns in a file
fix_matrix_returns() {
    local file=$1
    echo "Checking $file..."
    
    # Create a temporary file
    local temp_file="${file}.tmp"
    
    # Process the file
    perl -pe '
        # Match Matrix getter methods with simple return statements
        if (/^\s*matrix::Matrix\s+\w+\([^)]*\)\s*(?:const\s*)?\{/) {
            $in_getter = 1;
        }
        if ($in_getter && /^\s*return\s+([A-Za-z_]\w*)\s*;/) {
            my $var = $1;
            # Skip if already using explicit constructor or if returning literals
            unless ($var =~ /^(matrix|Matrix|true|false|nullptr|\d+)/ || /matrix::Matrix\(/) {
                s/return\s+$var\s*;/return matrix::Matrix($var);/;
            }
        }
        if (/^\s*\}/) {
            $in_getter = 0;
        }
    ' "$file" > "$temp_file"
    
    # Check if changes were made
    if ! cmp -s "$file" "$temp_file"; then
        mv "$temp_file" "$file"
        echo "  Fixed: $file"
    else
        rm "$temp_file"
    fi
}

# Process the files
for file in selforg/controller/{remotecontrolled,multilayerffnn,crossmotorcoupling,onelayerffnn,controllernet,replaycontroller,homeokinbase,derlinunivers,universalcontroller}.h; do
    if [ -f "$file" ]; then
        fix_matrix_returns "$file"
    fi
done

echo "Matrix return fixes completed."