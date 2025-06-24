#!/bin/bash
# Monitor build progress and handle errors for lpzrobots

# Configuration
BUILD_LOG_DIR="build_logs"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
BUILD_LOG="$BUILD_LOG_DIR/build_${TIMESTAMP}.log"
ERROR_LOG="$BUILD_LOG_DIR/errors_${TIMESTAMP}.log"
WARNING_LOG="$BUILD_LOG_DIR/warnings_${TIMESTAMP}.log"

# Create log directory
mkdir -p "$BUILD_LOG_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    
    case $status in
        "SUCCESS")
            echo -e "${GREEN}[SUCCESS]${NC} $message"
            ;;
        "ERROR")
            echo -e "${RED}[ERROR]${NC} $message"
            ;;
        "WARNING")
            echo -e "${YELLOW}[WARNING]${NC} $message"
            ;;
        *)
            echo "[INFO] $message"
            ;;
    esac
}

# Function to extract and log warnings
extract_warnings() {
    local log_file=$1
    local component=$2
    
    if grep -q "warning:" "$log_file"; then
        echo "=== Warnings for $component ===" >> "$WARNING_LOG"
        grep "warning:" "$log_file" >> "$WARNING_LOG"
        echo "" >> "$WARNING_LOG"
        
        local warning_count=$(grep -c "warning:" "$log_file")
        print_status "WARNING" "$warning_count warnings found in $component"
    fi
}

# Function to monitor and build a component
monitor_build() {
    local component=$1
    local make_target=${2:-$component}
    local temp_log="$BUILD_LOG_DIR/${component}_build.log"
    
    print_status "INFO" "Building $component..."
    echo "=== Building $component at $(date) ===" >> "$BUILD_LOG"
    
    # Run the build
    if make $make_target > "$temp_log" 2>&1; then
        print_status "SUCCESS" "$component built successfully"
        cat "$temp_log" >> "$BUILD_LOG"
        
        # Check for warnings even in successful builds
        extract_warnings "$temp_log" "$component"
        
        # Attempt to install
        if [ "$component" != "ode" ]; then  # ODE has special installation
            print_status "INFO" "Installing $component..."
            if make install_$component >> "$temp_log" 2>&1; then
                print_status "SUCCESS" "$component installed successfully"
            else
                print_status "ERROR" "Failed to install $component"
                tail -50 "$temp_log" >> "$ERROR_LOG"
                return 1
            fi
        fi
    else
        print_status "ERROR" "Build failed for $component"
        echo "=== Error in $component ===" >> "$ERROR_LOG"
        tail -100 "$temp_log" >> "$ERROR_LOG"
        
        # Try to identify common errors
        if grep -q "No such file or directory" "$temp_log"; then
            print_status "INFO" "Missing file detected, checking dependencies..."
        elif grep -q "undefined reference" "$temp_log"; then
            print_status "INFO" "Linker error detected, checking library paths..."
        elif grep -q "error:" "$temp_log"; then
            # Extract specific errors
            echo "Compilation errors:" >> "$ERROR_LOG"
            grep "error:" "$temp_log" | head -10 >> "$ERROR_LOG"
        fi
        
        return 1
    fi
    
    return 0
}

# Function to run pre-build checks
pre_build_checks() {
    print_status "INFO" "Running pre-build checks..."
    
    # Check for required tools
    for tool in make g++ m4 perl; do
        if ! command -v $tool &> /dev/null; then
            print_status "ERROR" "Required tool '$tool' not found"
            return 1
        fi
    done
    
    # Check for configuration
    if [ ! -f "Makefile.conf" ]; then
        print_status "ERROR" "Makefile.conf not found. Run configuration first."
        return 1
    fi
    
    # Check disk space (need at least 1GB)
    if command -v df &> /dev/null; then
        local available=$(df -k . | awk 'NR==2 {print $4}')
        if [ "$available" -lt 1048576 ]; then
            print_status "WARNING" "Low disk space: less than 1GB available"
        fi
    fi
    
    return 0
}

# Function to generate build report
generate_report() {
    local report_file="$BUILD_LOG_DIR/build_report_${TIMESTAMP}.txt"
    
    echo "=== LPZRobots Build Report ===" > "$report_file"
    echo "Date: $(date)" >> "$report_file"
    echo "" >> "$report_file"
    
    # Count warnings and errors
    local total_warnings=$(grep -c "warning:" "$BUILD_LOG" 2>/dev/null || echo 0)
    local total_errors=$(grep -c "error:" "$BUILD_LOG" 2>/dev/null || echo 0)
    
    echo "Summary:" >> "$report_file"
    echo "  Total Warnings: $total_warnings" >> "$report_file"
    echo "  Total Errors: $total_errors" >> "$report_file"
    echo "" >> "$report_file"
    
    # List built components
    echo "Components Status:" >> "$report_file"
    for comp in utils selforg ode ode_robots ga_tools; do
        if grep -q "SUCCESS.*$comp built successfully" "$BUILD_LOG"; then
            echo "  ✓ $comp: SUCCESS" >> "$report_file"
        else
            echo "  ✗ $comp: FAILED" >> "$report_file"
        fi
    done
    
    echo "" >> "$report_file"
    echo "Log files:" >> "$report_file"
    echo "  Build log: $BUILD_LOG" >> "$report_file"
    echo "  Error log: $ERROR_LOG" >> "$report_file"
    echo "  Warning log: $WARNING_LOG" >> "$report_file"
    
    print_status "INFO" "Build report saved to: $report_file"
}

# Main monitoring function
main() {
    print_status "INFO" "LPZRobots Build Monitor Started"
    
    # Run pre-build checks
    if ! pre_build_checks; then
        print_status "ERROR" "Pre-build checks failed"
        exit 1
    fi
    
    # Build sequence
    local components=("utils" "selforg" "ode" "ode_robots" "ga_tools")
    local failed=0
    
    for component in "${components[@]}"; do
        if ! monitor_build "$component"; then
            print_status "ERROR" "Build failed at $component"
            failed=1
            break
        fi
    done
    
    # Generate report
    generate_report
    
    if [ $failed -eq 0 ]; then
        print_status "SUCCESS" "All components built successfully!"
        print_status "INFO" "Check $WARNING_LOG for warnings that should be addressed"
    else
        print_status "ERROR" "Build failed. Check logs for details."
        exit 1
    fi
}

# Run main function if script is executed directly
if [ "${BASH_SOURCE[0]}" == "${0}" ]; then
    main "$@"
fi