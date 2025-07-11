# LPZRobots Config Scripts Analysis

## Overview
The LPZRobots build system uses `*-config` scripts similar to `pkg-config` to provide compilation and linking flags for each component. These scripts are generated from M4 templates during configuration.

## Config Scripts Found

### Core Config Scripts
1. **selforg-config** - Provides flags for the selforg controller library
   - Template: `selforg/selforg-config.m4`
   - Generated: `selforg/selforg-config`
   - Purpose: Provides include paths, library paths, and dependencies (GSL, readline, ncurses)

2. **ode_robots-config** - Provides flags for the ode_robots simulation framework
   - Template: `ode_robots/ode_robots-config.m4`
   - Generated: `ode_robots/ode_robots-config`
   - Purpose: Provides OSG, OpenGL, ODE dependencies

3. **ga_tools-config** - Provides flags for genetic algorithm tools
   - Template: `ga_tools/ga_tools-config.m4`
   - Generated: `ga_tools/ga_tools-config`

4. **configurator-config** - Provides flags for the configurator GUI
   - Template: `configurator/configurator-config.m4`
   - Generated: `configurator/configurator-config`

### Special Config Scripts
5. **ode-dbl-config** - Wrapper for ODE physics engine
   - Location: `ode_robots/ode-dbl-config`
   - Purpose: Points to the actual ODE config (either system or local build)

## How Config Scripts Work

### 1. Generation Process
```bash
# During configure phase
m4 -DPREFIX="$prefix" -DSRCPREFIX="$srcprefix" -DMAC -DUSER component-config.m4 > component-config
chmod +x component-config
```

### 2. Usage in Build System
```makefile
# In simulation Makefiles
SELFORGCFG := $(shell if [ -x ../../../selforg/selforg-config ]; then echo "../../../selforg/selforg-config"; else echo "selforg-config"; fi)
BASELIBS = $(shell $(SELFORGCFG) $(CFGOPTS) --libs)
CPPFLAGS = $(shell $(SELFORGCFG) $(CFGOPTS) --cflags)
```

### 3. Common Options
- `--cflags`: Compiler flags and include paths
- `--libs`: Linker flags and library dependencies
- `--prefix`: Installation prefix
- `--srcprefix`: Source directory (for development builds)
- `--opt/--dbg`: Build variant selection
- `--static`: Force static linking
- `--type`: Returns USER or DEVEL

## Directory Structure and PATH Requirements

### Self-Contained Build System
The build system is designed to work without system-wide installation:

```
lpzrobots/
├── selforg/
│   ├── selforg-config          # Generated from M4 template
│   └── selforg-config.m4       # M4 template
├── ode_robots/
│   ├── ode_robots-config       # Generated from M4 template
│   ├── ode_robots-config.m4    # M4 template
│   ├── ode-dbl-config          # Wrapper script
│   ├── configure               # Looks for ../selforg/selforg-config first
│   └── simulations/
│       └── template_sphererobot/
│           └── Makefile        # Uses ../../../selforg/selforg-config
└── ga_tools/
    ├── ga_tools-config
    └── configure               # Also uses ../selforg/selforg-config
```

### Path Resolution Strategy

1. **Local First**: Components first check for config scripts relative to their location
   - ode_robots/configure checks `../selforg/selforg-config`
   - ga_tools/configure checks `../selforg/selforg-config`
   - Simulations check `../../../selforg/selforg-config`

2. **PATH Fallback**: If not found locally, falls back to system PATH
   - This allows for system-wide installations
   - But local development takes precedence

3. **PATH Updates**: When local scripts are found, they update PATH
   ```bash
   if [ -x ../selforg/selforg-config ]; then
       export PATH="../selforg:$PATH"
   fi
   ```

## CMake vs Legacy Make Comparison

### Legacy Make System
- Uses M4 templates to generate shell scripts
- Config scripts provide flags via command substitution
- Supports both development and user installations
- Platform-specific logic embedded in M4 macros

### CMake System
- Uses modern CMake targets and properties
- No config scripts needed - uses target dependencies
- Better IDE integration and cross-platform support
- Config scripts generated for backward compatibility

### CMake Config Script Generation
```cmake
# In cmake/selforg-config.sh.in
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/selforg-config.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/selforg-config
    @ONLY
)
```

## Key Insights

1. **No System Installation Required**: The build system is designed to work entirely within the source tree
2. **Relative Path Priority**: Local components always take precedence over system installations
3. **Cascading Dependencies**: ode_robots depends on selforg, ga_tools depends on selforg
4. **Platform Abstraction**: M4 macros handle Linux/macOS differences
5. **Development vs User Mode**: DEVEL mode uses source paths, USER mode uses installation paths

## Common Issues and Solutions

### Issue: "could not find selforg-config in PATH or ../selforg/"
**Cause**: The configure script can't find selforg-config
**Solution**: 
1. Ensure selforg has been configured first: `cd selforg && ./configure`
2. Check that selforg-config has execute permissions
3. Run configure from the correct directory

### Issue: Missing dependencies
**Cause**: Config scripts can't find required libraries
**Solution**: Install system dependencies or build local versions