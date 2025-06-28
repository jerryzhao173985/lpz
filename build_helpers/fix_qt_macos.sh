#!/bin/bash
# Fix Qt/macOS issues for lpzrobots build system
# This script handles common Qt-related build issues on macOS

echo "=== Qt/macOS Build Fixer ==="

# Function to fix AGL framework references in a directory
fix_agl_framework() {
    local dir="$1"
    echo "Fixing AGL framework references in: $dir"
    
    # Find and fix all Makefiles
    find "$dir" -name "Makefile*" -type f | while read -r makefile; do
        if grep -q "framework AGL" "$makefile" 2>/dev/null; then
            # Remove all AGL references
            sed -i '' -e 's/-framework AGL//g' \
                      -e 's|-F/System/Library/Frameworks/AGL.framework||g' \
                      -e 's|-I/System/Library/Frameworks/AGL.framework/Headers||g' \
                      "$makefile"
            echo "  Fixed: $makefile"
        fi
    done
    
    # Also check for any .xcodeproj files
    find "$dir" -name "*.xcodeproj" -type d | while read -r xcodeproj; do
        find "$xcodeproj" -type f | while read -r projfile; do
            if grep -q "AGL.framework" "$projfile" 2>/dev/null; then
                sed -i '' 's/AGL\.framework[^;]*;//g' "$projfile"
                echo "  Fixed Xcode project: $projfile"
            fi
        done
    done
}

# Function to configure a Qt project with proper macOS settings
configure_qt_project() {
    local project_dir="$1"
    local pro_file="$2"
    
    echo "Configuring Qt project: $project_dir/$pro_file"
    
    cd "$project_dir" || exit 1
    
    # Run qmake with explicit OpenGL override
    qmake -makefile "$pro_file" "QMAKE_LIBS_OPENGL=-framework OpenGL" "CONFIG-=opengl"
    
    # Fix any AGL references that might have been generated
    fix_agl_framework "."
    
    cd - > /dev/null
}

# Function to suppress external library warnings
add_warning_suppression() {
    local makefile="$1"
    
    if [ -f "$makefile" ]; then
        echo "Adding warning suppression to: $makefile"
        
        # Add -isystem flags for external includes if not already present
        if ! grep -q "isystem /opt/homebrew/include" "$makefile"; then
            sed -i '' 's|INCPATH\s*=|INCPATH = -isystem /opt/homebrew/include |' "$makefile"
        fi
    fi
}

# Main execution
if [ $# -eq 0 ]; then
    echo "Usage: $0 <command> [args]"
    echo "Commands:"
    echo "  fix-agl <directory>     - Remove AGL framework references from directory"
    echo "  configure <dir> <.pro>  - Configure Qt project with macOS fixes"
    echo "  suppress-warnings <makefile> - Add external library warning suppression"
    echo "  fix-all                 - Fix all known Qt projects in lpzrobots"
    exit 1
fi

case "$1" in
    fix-agl)
        fix_agl_framework "${2:-.}"
        ;;
    configure)
        configure_qt_project "$2" "$3"
        ;;
    suppress-warnings)
        add_warning_suppression "$2"
        ;;
    fix-all)
        # Fix all known Qt projects
        echo "Fixing all Qt projects..."
        
        # guilogger
        if [ -d "guilogger" ]; then
            configure_qt_project "guilogger" "guilogger.pro"
        fi
        
        # matrixviz
        if [ -d "matrixviz" ]; then
            configure_qt_project "matrixviz" "matrixviz.pro"
        fi
        
        # configurator
        if [ -d "configurator" ]; then
            echo "Note: configurator should be built after selforg"
            configure_qt_project "configurator" "configurator.pro"
        fi
        ;;
    *)
        echo "Unknown command: $1"
        exit 1
        ;;
esac

echo "Done."