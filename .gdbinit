# GDB initialization file for LPZRobots debugging
# Place in project root or copy to ~/.gdbinit

# Security - allow project-specific .gdbinit
set auto-load safe-path /Users/jerry/simulator/lpz

# Better formatting
set print pretty on
set print array on
set print array-indexes on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set print asm-demangle on
set pagination off
set confirm off

# History
set history save on
set history size 10000
set history filename ~/.gdb_history

# Debugging helpers
set breakpoint pending on
set backtrace limit 50

# STL pretty printers (if available)
python
import sys
import os
sys.path.insert(0, '/usr/share/gcc/python')
try:
    from libstdcxx.v6.printers import register_libstdcxx_printers
    register_libstdcxx_printers(None)
    print("STL pretty printers loaded")
except ImportError:
    print("STL pretty printers not available")
end

# Custom commands for LPZRobots debugging

# Print matrix contents nicely
define pmatrix
    if $argc == 0
        help pmatrix
    else
        set $mat = $arg0
        set $rows = $mat.m
        set $cols = $mat.n
        printf "Matrix %dx%d:\n", $rows, $cols
        set $i = 0
        while $i < $rows
            set $j = 0
            printf "["
            while $j < $cols
                printf " %7.4f", $mat.data[$i * $cols + $j]
                set $j = $j + 1
            end
            printf " ]\n"
            set $i = $i + 1
        end
    end
end
document pmatrix
Print a matrix in readable format
Usage: pmatrix <matrix_variable>
end

# Print robot sensor values
define psensors
    if $argc == 0
        help psensors
    else
        set $robot = $arg0
        set $num = $robot->getSensorNumber()
        printf "Robot sensors (%d):\n", $num
        set $i = 0
        while $i < $num
            printf "  [%2d]: %7.4f\n", $i, $robot->sensors[$i]
            set $i = $i + 1
        end
    end
end
document psensors
Print robot sensor values
Usage: psensors <robot_pointer>
end

# Print robot motor values
define pmotors
    if $argc == 0
        help pmotors
    else
        set $robot = $arg0
        set $num = $robot->getMotorNumber()
        printf "Robot motors (%d):\n", $num
        set $i = 0
        while $i < $num
            printf "  [%2d]: %7.4f\n", $i, $robot->motors[$i]
            set $i = $i + 1
        end
    end
end
document pmotors
Print robot motor values
Usage: pmotors <robot_pointer>
end

# Break on common error conditions
define lpz_catch_errors
    catch throw
    break abort
    break __assert_fail
    break matrix::Matrix::val if i >= m || j >= n
    printf "Error catching enabled\n"
end
document lpz_catch_errors
Set breakpoints on common error conditions in LPZRobots
end

# Useful breakpoints
define lpz_break_sim
    break Simulation::start
    break Simulation::loop
    break OdeAgent::step
    break AbstractController::step
    printf "Simulation breakpoints set\n"
end
document lpz_break_sim
Set breakpoints at key simulation points
end

# Print agent info
define pagent
    if $argc == 0
        help pagent
    else
        set $agent = $arg0
        printf "Agent: %s\n", $agent->getName().c_str()
        printf "  Robot: %s\n", $agent->getRobot()->getName().c_str()
        printf "  Controller: %s\n", $agent->getController()->getName().c_str()
        printf "  Step: %ld\n", $agent->t
    end
end
document pagent
Print OdeAgent information
Usage: pagent <agent_pointer>
end

# Aliases
alias -a pv = print
alias -a pd = print -pretty
alias -a ph = print/x
alias -a pb = print/t

# Catchpoints for debugging
catch throw std::runtime_error
catch throw std::out_of_range
catch throw std::invalid_argument

# Load symbols for all shared libraries
set solib-search-path ./selforg:./ode_robots:./opende:./guilogger:./matrixviz

# Startup message
printf "LPZRobots GDB configuration loaded\n"
printf "Useful commands:\n"
printf "  pmatrix <mat>  - Print matrix contents\n"
printf "  psensors <bot> - Print robot sensors\n"
printf "  pmotors <bot>  - Print robot motors\n"
printf "  pagent <agt>   - Print agent info\n"
printf "  lpz_catch_errors - Enable error catching\n"
printf "  lpz_break_sim  - Set simulation breakpoints\n"