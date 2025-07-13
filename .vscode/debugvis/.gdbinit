# LPZRobots GDB initialization file
# This file is automatically loaded by VSCode debug configurations

# Load the LPZRobots debug pretty printers
python
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
try:
    import lpzrobots_debugvis
    print("LPZRobots GDB formatters loaded successfully!")
except Exception as e:
    print(f"Failed to load LPZRobots formatters: {e}")
end

# Enable pretty printing
set print pretty on
set print array on
set print array-indexes on
set print elements 200

# STL pretty printers (if available)
python
try:
    from libstdcxx.v6.printers import register_libstdcxx_printers
    register_libstdcxx_printers(None)
    print("STL pretty printers loaded")
except ImportError:
    pass
end

# Useful commands for matrix debugging
define matrix
    if $argc == 0
        help matrix
    else
        print $arg0
        printf "Matrix[%d x %d]:\n", $arg0.m, $arg0.n
        set $rows = $arg0.m
        set $cols = $arg0.n
        set $i = 0
        while $i < $rows
            printf "  ["
            set $j = 0
            while $j < $cols
                printf " %.4g", $arg0.data[$i * $cols + $j]
                set $j = $j + 1
            end
            printf " ]\n"
            set $i = $i + 1
        end
    end
end
document matrix
Print a matrix in a readable format.
Usage: matrix <matrix_variable>
end

# Command to show robot state
define robot
    if $argc == 0
        help robot
    else
        print $arg0
        printf "Robot: %s\n", $arg0.name
        printf "  Sensors: %d\n", $arg0.sensorno
        printf "  Motors: %d\n", $arg0.motorno
        if $arg0.sensors && $arg0.sensorno > 0
            printf "  Sensor values: ["
            set $i = 0
            while $i < $arg0.sensorno && $i < 10
                printf " %.3f", $arg0.sensors[$i]
                set $i = $i + 1
            end
            if $arg0.sensorno > 10
                printf " ..."
            end
            printf " ]\n"
        end
    end
end
document robot
Print robot state information.
Usage: robot <robot_variable>
end

# Shortcuts
alias m = next
alias n = step
alias c = continue
alias f = finish

echo "LPZRobots debugging helpers loaded. Use 'matrix' and 'robot' commands.\n"