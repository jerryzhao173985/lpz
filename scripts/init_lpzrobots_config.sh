#!/bin/bash
# Script to initialize LPZRobots configuration directory and files

CONFIG_DIR="$HOME/.lpzrobots"

echo "Initializing LPZRobots configuration..."

# Create configuration directory
if [ ! -d "$CONFIG_DIR" ]; then
    echo "Creating configuration directory: $CONFIG_DIR"
    mkdir -p "$CONFIG_DIR"
else
    echo "Configuration directory already exists: $CONFIG_DIR"
fi

# Create default ode_robots.cfg if it doesn't exist
if [ ! -f "$CONFIG_DIR/ode_robots.cfg" ]; then
    echo "Creating default ode_robots.cfg..."
    cat > "$CONFIG_DIR/ode_robots.cfg" << 'EOF'
# ODE Robots Configuration File
# This file contains default settings for ODE robot simulations

[General]
windowsize=800x600
fps=25
realtime=1

[Graphics]
noshadow=0
shadowsize=2048
drawboundings=0

[Simulation]
noise=0.1
controlinterval=1
EOF
    echo "Created: $CONFIG_DIR/ode_robots.cfg"
else
    echo "ode_robots.cfg already exists"
fi

# Create default guilogger.cfg if it doesn't exist
if [ ! -f "$CONFIG_DIR/guilogger.cfg" ]; then
    echo "Creating default guilogger.cfg..."
    cat > "$CONFIG_DIR/guilogger.cfg" << 'EOF'
[General]
Version=V0.7
PlotWindows=5
UpdateInterval=2000
MinData4Replot=1
Gnuplot=gnuplot

[PlotWindow0]
Visible=true
MinimalMode=false
Number=0
XInterval=500
YInterval=-2,2
Channels=
EOF
    echo "Created: $CONFIG_DIR/guilogger.cfg"
else
    echo "guilogger.cfg already exists"
fi

# Create default matrixviz.cfg if it doesn't exist
if [ ! -f "$CONFIG_DIR/matrixviz.cfg" ]; then
    echo "Creating default matrixviz.cfg..."
    cat > "$CONFIG_DIR/matrixviz.cfg" << 'EOF'
[General]
Version=1.0
ColorPalette=Default
MinValue=-1.0
MaxValue=1.0

[Display]
ShowGrid=true
ShowLabels=true
CellSize=20
EOF
    echo "Created: $CONFIG_DIR/matrixviz.cfg"
else
    echo "matrixviz.cfg already exists"
fi

echo ""
echo "Configuration initialization complete!"
echo "Configuration files are located in: $CONFIG_DIR"
echo ""
echo "You can now run:"
echo "  guilogger - for plotting sensor/motor data"
echo "  matrixviz - for visualizing matrices"