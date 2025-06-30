#!/bin/bash
# Modernize dependency generation to use native macOS clang++ instead of makedepend
# This is the best practice for macOS ARM64 (M1/M2/M3/M4)

echo "Modernizing dependency generation for native macOS ARM64..."

# Function to convert a Makefile to use modern dependency generation
modernize_makefile() {
    local makefile="$1"
    local backup="${makefile}.bak.$(date +%Y%m%d_%H%M%S)"
    
    if [ ! -f "$makefile" ]; then
        echo "Warning: $makefile not found"
        return
    fi
    
    echo "Processing $makefile..."
    cp "$makefile" "$backup"
    
    # Check if it uses makedepend
    if grep -q "makedepend" "$makefile"; then
        echo "  Found makedepend usage - modernizing..."
        
        # Create a temporary file with the modern approach
        cat > "${makefile}.modern" << 'EOF'
# Modern dependency generation for macOS ARM64
# Using native clang++ with -MMD -MP flags

# Add dependency flags to compilation
DEPFLAGS = -MMD -MP
CPPFLAGS += $(DEPFLAGS)

# Get list of dependency files from object files
DEPS := $(OFILES:.o=.d)

# Include the dependency files
-include $(DEPS)

# Clean also removes dependency files
clean: clean-deps

clean-deps:
	rm -f $(DEPS)

# Remove old makedepend-based rules
# Makefile.depend target removed - dependencies now generated during compilation
# depend target removed - no longer needed with automatic dependency generation
EOF
        
        # Apply the modernization
        # 1. Remove or comment out makedepend-related lines
        sed -i '' \
            -e '/^Makefile\.depend:/,/^[^\t]/{/^[^\t]/!d; /^Makefile\.depend:/d;}' \
            -e '/^depend:/,/^[^\t]/{/^[^\t]/!d; /^depend:/d;}' \
            -e 's/makedepend/# makedepend (replaced with -MMD -MP)/' \
            -e '/-include Makefile\.depend/d' \
            "$makefile"
        
        # 2. Add modern dependency handling after the BUILD_DIR definition
        awk '
        /^BUILD_DIR/ { 
            print
            print ""
            print "# Modern dependency generation"
            print "DEPFLAGS = -MMD -MP"
            print "DEPS := $(OFILES:.o=.d)"
            print ""
            next
        }
        { print }
        ' "$makefile" > "${makefile}.tmp" && mv "${makefile}.tmp" "$makefile"
        
        # 3. Update the compilation rule to use DEPFLAGS
        sed -i '' \
            -e 's/\$(CXX) -c \$(CPPFLAGS)/$(CXX) -c $(CPPFLAGS) $(DEPFLAGS)/' \
            "$makefile"
        
        # 4. Add -include for dependency files
        echo "" >> "$makefile"
        echo "# Include generated dependency files" >> "$makefile"
        echo "-include \$(DEPS)" >> "$makefile"
        
        # 5. Update clean target
        sed -i '' \
            -e '/^clean:/a\
	rm -f $(DEPS)' \
            "$makefile"
        
        echo "  Modernization complete!"
    else
        echo "  Already using modern dependency generation"
    fi
}

# Function to create a modern Makefile template
create_modern_template() {
    cat > "Makefile.modern.template" << 'EOF'
# Modern Makefile template for macOS ARM64
# Uses native clang++ dependency generation

CXX = clang++
CXXFLAGS = -std=c++17 -Wall -O2
DEPFLAGS = -MMD -MP
CPPFLAGS = $(CXXFLAGS) $(DEPFLAGS) $(INC)

# Source files
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPS = $(SOURCES:.cpp=.d)

# Build directory support
BUILD_DIR = build
OBJECTS := $(addprefix $(BUILD_DIR)/, $(OBJECTS))
DEPS := $(addprefix $(BUILD_DIR)/, $(DEPS))

# Main target
all: myprogram

myprogram: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compilation rule with automatic dependency generation
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Include generated dependencies
-include $(DEPS)

# Clean targets
clean:
	rm -rf $(BUILD_DIR) myprogram

.PHONY: all clean
EOF
    echo "Created Makefile.modern.template with best practices"
}

# Check if we're on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "Warning: This script is designed for macOS but running on $OSTYPE"
fi

# Check compiler
echo "Checking compiler..."
echo "  CXX = $(which clang++ || which g++)"
echo "  Version: $(clang++ --version 2>/dev/null | head -1 || g++ --version | head -1)"

# Check if makedepend is even available
if which makedepend >/dev/null 2>&1; then
    echo "  makedepend found at: $(which makedepend)"
    echo "  Note: makedepend is from Homebrew, not native to macOS"
else
    echo "  makedepend not found (this is normal on macOS)"
fi

# Demonstration of the flags
echo ""
echo "Modern dependency generation flags explained:"
echo "  -MMD : Generate .d file with user header dependencies (excludes system headers)"
echo "  -MP  : Add phony targets for headers (prevents errors if headers are deleted)"
echo "  -MF  : Specify dependency filename (optional, defaults to source.d)"
echo ""
echo "Example usage:"
echo "  clang++ -c main.cpp -MMD -MP -o main.o"
echo "  This creates both main.o and main.d"
echo ""

# Show what a .d file looks like
echo "Example .d file content:"
cat << 'EOF'
main.o: main.cpp robot.h controller.h physics.h
robot.h:
controller.h:
physics.h:
EOF

echo ""
echo "The empty rules for headers prevent 'No rule to make target' errors"
echo "if a header file is deleted or renamed."

# Offer to modernize specific Makefiles
if [ $# -gt 0 ]; then
    for makefile in "$@"; do
        modernize_makefile "$makefile"
    done
else
    echo ""
    echo "Usage: $0 [Makefile ...]"
    echo "  Modernizes specified Makefiles to use native clang++ dependency generation"
    echo ""
    create_modern_template
fi