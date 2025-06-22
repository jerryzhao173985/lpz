# LPZRobots GUI Tools Examples

This document provides examples and usage instructions for the LPZRobots GUI tools (guilogger and matrixviz) on macOS ARM64.

## Prerequisites

1. Ensure Qt5 is installed:
   ```bash
   brew install qt@5
   ```

2. Initialize configuration (if not already done):
   ```bash
   ./init_lpzrobots_config.sh
   ```

3. Set up PATH for GUI tools:
   ```bash
   source ./setup_gui_path.sh
   ```
   
   Or add to your shell configuration (~/.zshrc):
   ```bash
   export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
   ```

## guilogger

guilogger is a real-time plotting tool for visualizing sensor and motor data from robot simulations.

### Usage Modes

#### 1. File Mode (for viewing saved data)
```bash
# View data from a file
./guilogger/guilogger-wrapper.sh -m file -f data.txt

# Example data format (data.txt):
#C sensor1[rad]
0.1
0.2
0.3
#C motor1[N]
1.0
0.9
0.8
#RESET
```

#### 2. Pipe Mode (real-time streaming)
```bash
# Stream data from a simulation
./ode_robots/simulations/template_sphererobot/start -g 1 | ./guilogger/guilogger-wrapper.sh -m pipe

# Or use the simulation's built-in option
./ode_robots/simulations/template_sphererobot/start -g 1
```

#### 3. File Pipe Mode (replay with timing)
```bash
# Replay data with delays
./guilogger/guilogger-wrapper.sh -m fpipe -f data.txt -d 100
```

### Data Format

guilogger expects data in the following format:
- `#C channelname[unit]` - Define a new channel
- Numeric values - Data points for the most recent channel
- `#RESET` - Reset all channels
- `#QUIT` - Close guilogger

## matrixviz

matrixviz is a real-time matrix visualization tool for viewing weight matrices and other 2D data.

### Usage

```bash
# Stream matrix data
./simulation | ./matrixviz/matrixviz-wrapper.sh -novideo

# Example data format:
#M matrix_name rows cols
value1 value2 ...
value3 value4 ...
```

### Options
- `-novideo` - Don't record video
- `-file filename` - Save matrices to file
- `-step` - Step through matrices manually

### Data Format

matrixviz expects data in the following format:
- `#M name rows cols` - Define a matrix with dimensions
- Space-separated values following the header
- Multiple matrices can be sent sequentially

## Integration with Simulations

### Running a Simulation with GUI Tools

1. **With guilogger only:**
   ```bash
   cd ode_robots/simulations/template_sphererobot
   ./start -g 1
   ```

2. **With matrixviz only:**
   ```bash
   cd ode_robots/simulations/template_sphererobot
   ./start -m 1
   ```

3. **With both tools:**
   ```bash
   cd ode_robots/simulations/template_sphererobot
   ./start -g 1 -m 1
   ```

### Command Line Options for Simulations

- `-g interval` - Enable guilogger with update interval (default: 1)
- `-m interval` - Enable matrixviz with update interval (default: 10)
- `-f interval filter name` - Log data to file

## Troubleshooting

### guilogger/matrixviz crashes on startup

1. Ensure configuration directory exists:
   ```bash
   ls -la ~/.lpzrobots/
   ```

2. Re-run permission fix:
   ```bash
   ./fix_gui_permissions.sh
   ```

3. Check Qt installation:
   ```bash
   brew list qt@5
   ```

### No data displayed

1. Verify data format is correct
2. Check that simulation is producing output:
   ```bash
   ./start -g 1 2>&1 | grep "#C"
   ```

### GUI doesn't appear

1. Ensure you're running on a system with display access
2. Check Qt environment:
   ```bash
   echo $QT_PLUGIN_PATH
   ```

## Example Scripts

### Generate Test Data for guilogger
```bash
#!/bin/bash
cat << EOF
#C sin_wave[rad]
#C cos_wave[rad]
EOF

for i in {1..100}; do
    t=$(echo "scale=2; $i * 0.1" | bc)
    sin=$(echo "s($t)" | bc -l)
    cos=$(echo "c($t)" | bc -l)
    echo $sin
    echo $cos
    sleep 0.1
done
echo "#QUIT"
```

### Generate Test Data for matrixviz
```bash
#!/bin/bash
for i in {1..50}; do
    echo "#M test_matrix 3 3"
    for j in {1..9}; do
        echo -n "$(echo "scale=2; $RANDOM / 32768" | bc) "
    done
    echo
    sleep 0.2
done
```

## Advanced Usage

### Custom Configuration

Edit configuration files in `~/.lpzrobots/`:
- `guilogger.cfg` - guilogger settings
- `matrixviz.cfg` - matrixviz settings
- `ode_robots.cfg` - simulation settings

### Recording Videos

matrixviz can record videos of matrix visualizations:
```bash
./simulation | ./matrixviz/matrixviz-wrapper.sh -file output.mv
```

### Filtering Channels

guilogger supports filtering channels:
```bash
./start -g 1 "{+sensor -motor}"  # Show only sensor channels
```

## PATH Setup

To use the tools from anywhere, add to your shell configuration:
```bash
# Add to ~/.zshrc or ~/.bashrc
export PATH=$PATH:/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz

# Create convenience aliases
alias guilogger="/Users/jerry/lpzrobots_mac/guilogger/guilogger-wrapper.sh"
alias matrixviz="/Users/jerry/lpzrobots_mac/matrixviz/matrixviz-wrapper.sh"
```