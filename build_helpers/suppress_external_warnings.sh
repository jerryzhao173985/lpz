#!/bin/bash
# Suppress warnings from external libraries in lpzrobots build system

echo "=== External Library Warning Suppressor ==="

# Function to convert -I flags to -isystem for external paths
convert_to_isystem() {
    local input="$1"
    local output=""
    
    # Convert common external library paths to -isystem
    output=$(echo "$input" | sed -E \
        -e 's/-I([^ ]*\/homebrew[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/Cellar[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/opt\/local[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/usr\/local[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/MacOSX.*\.sdk[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/Frameworks[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/Qt[^ ]*)/-isystem \1/g')
    
    echo "$output"
}

# Function to update a Makefile to use -isystem
update_makefile() {
    local makefile="$1"
    
    if [ ! -f "$makefile" ]; then
        echo "Error: $makefile not found"
        return 1
    fi
    
    echo "Updating: $makefile"
    
    # Backup the original
    cp "$makefile" "${makefile}.bak"
    
    # Update CPPFLAGS/CXXFLAGS/INCPATH lines
    sed -i '' -E \
        -e 's/-I([^ ]*\/homebrew[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/Cellar[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/opt\/local[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/usr\/local[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/MacOSX.*\.sdk[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/Frameworks[^ ]*)/-isystem \1/g' \
        -e 's/-I([^ ]*\/Qt[^ ]*)/-isystem \1/g' \
        "$makefile"
    
    echo "  Converted external includes to -isystem"
}

# Function to update config scripts
update_config_script() {
    local script="$1"
    
    if [ ! -f "$script" ]; then
        echo "Error: $script not found"
        return 1
    fi
    
    echo "Updating config script: $script"
    
    # Check if already has -isystem conversion
    if grep -q "sed.*isystem" "$script"; then
        echo "  Already has -isystem conversion"
        return 0
    fi
    
    # Add -isystem conversion to CFLAGS output
    sed -i '' '/--cflags)/,/;;/{
        s/echo \$CPPFLAGS/echo \$CPPFLAGS | sed -E '\''s\/-I(\[^ \]*\\\/homebrew\[^ \]*)\/-isystem \\1\/g; s\/-I(\[^ \]*\\\/Cellar\[^ \]*)\/-isystem \\1\/g'\''/
    }' "$script"
    
    echo "  Added -isystem conversion"
}

# Main execution
if [ $# -eq 0 ]; then
    echo "Usage: $0 <command> [args]"
    echo "Commands:"
    echo "  makefile <file>     - Update a Makefile to use -isystem"
    echo "  config <script>     - Update a config script to output -isystem"
    echo "  convert <flags>     - Convert -I flags to -isystem (for testing)"
    echo "  fix-all             - Fix all Makefiles and config scripts"
    exit 1
fi

case "$1" in
    makefile)
        update_makefile "$2"
        ;;
    config)
        update_config_script "$2"
        ;;
    convert)
        shift
        convert_to_isystem "$*"
        ;;
    fix-all)
        echo "Fixing all Makefiles and config scripts..."
        
        # Find and update all Makefiles
        find . -name "Makefile" -type f | while read -r makefile; do
            # Skip backup files and certain directories
            if [[ "$makefile" == *.bak ]] || [[ "$makefile" == */.git/* ]]; then
                continue
            fi
            update_makefile "$makefile"
        done
        
        # Update config scripts
        for script in */configure */*-config; do
            if [ -f "$script" ]; then
                update_config_script "$script"
            fi
        done
        ;;
    *)
        echo "Unknown command: $1"
        exit 1
        ;;
esac

echo "Done."