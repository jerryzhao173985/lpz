# LPZRobots GUI Tools Quick Start

## Initial Setup (One Time)

1. Initialize configuration:
   ```bash
   ./init_lpzrobots_config.sh
   ```

2. Add to your ~/.zshrc:
   ```bash
   export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
   ```

3. Reload shell:
   ```bash
   source ~/.zshrc
   ```

## Running Simulations with GUI

### Option 1: Source the setup script (temporary)
```bash
source ./setup_gui_path.sh
cd ode_robots/simulations/template_sphererobot
./start -g -m
```

### Option 2: If PATH is permanently set
```bash
cd ode_robots/simulations/template_sphererobot
./start -g -m
```

## Command Options

- `-g` or `-g 1` - Enable guilogger (plots sensor/motor data)
- `-m` or `-m 1` - Enable matrixviz (visualizes weight matrices)
- `-g -m` - Enable both tools
- `-nographics` - Run simulation without 3D visualization (headless)

## Troubleshooting

If you get "command not found" errors:
```bash
# Check if PATH is set:
echo $PATH | grep lpzrobots

# If not, source the setup script:
source /Users/jerry/lpzrobots_mac/setup_gui_path.sh

# Or run simulation with explicit PATH:
PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH" ./start -g -m
```

## Testing

Run the complete test:
```bash
./test_complete_gui.sh
```

This will verify that all GUI tools are working correctly.