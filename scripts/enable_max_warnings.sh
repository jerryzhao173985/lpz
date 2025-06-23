#!/bin/bash
# Script to enable maximum compiler warnings in all Makefiles

echo "=== Enabling Maximum Compiler Warnings ==="

# Define the warning flags we want to add
EXTRA_WARNINGS="-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wcast-align -Wformat=2 -Wuninitialized -Wunused-function -Wunused-label -Wunused-variable -Wunused-value -Wunused-parameter -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Woverloaded-virtual -Wsign-promo -Wformat-security"

# For C++17 specific warnings
CPP17_WARNINGS="-Wnon-virtual-dtor -Wold-style-cast -Wzero-as-null-pointer-constant"

# Warnings to explicitly disable (too noisy or not useful)
DISABLED_WARNINGS="-Wno-unused-parameter -Wno-sign-conversion"

# Combined warning flags
ALL_WARNINGS="$EXTRA_WARNINGS $CPP17_WARNINGS $DISABLED_WARNINGS"

# Function to update a Makefile
update_makefile() {
    local makefile=$1
    echo "Updating $makefile..."
    
    # Check if the file exists
    if [ ! -f "$makefile" ]; then
        echo "  File not found: $makefile"
        return
    fi
    
    # Check if we already added our warnings
    if grep -q "EXTRA_WARNINGS" "$makefile"; then
        echo "  Already updated"
        return
    fi
    
    # Create backup
    cp "$makefile" "$makefile.bak"
    
    # Add our warning flags after CPPFLAGS definition
    awk -v warnings="$ALL_WARNINGS" '
    /^CPPFLAGS/ && !done {
        print $0
        print "# Maximum warning flags added by enable_max_warnings.sh"
        print "EXTRA_WARNINGS = " warnings
        print "CPPFLAGS += $(EXTRA_WARNINGS)"
        done = 1
        next
    }
    { print }
    ' "$makefile" > "$makefile.tmp"
    
    # Replace original file
    mv "$makefile.tmp" "$makefile"
    echo "  Updated successfully"
}

# Update selforg Makefile
update_makefile "selforg/Makefile"

# Update ode_robots Makefile
update_makefile "ode_robots/Makefile"

# Update utils Makefiles
update_makefile "matrixviz/Makefile"
update_makefile "guilogger/Makefile"
update_makefile "configurator/Makefile"

# Update ga_tools Makefile
update_makefile "ga_tools/Makefile"

# Update opende Makefile if it exists
if [ -f "opende/Makefile" ]; then
    update_makefile "opende/Makefile"
fi

echo ""
echo "=== Maximum warnings enabled ==="
echo "Warning flags added:"
echo "  $ALL_WARNINGS"
echo ""
echo "To revert changes, restore from .bak files:"
echo "  find . -name '*.bak' -exec sh -c 'mv {} {%.bak}' \;"
echo ""
echo "Now run 'make clean && make' to see all warnings"