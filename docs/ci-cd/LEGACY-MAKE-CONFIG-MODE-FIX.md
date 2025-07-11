# Legacy Make Config Mode Fix

## Problem
The Legacy Make simulation test was failing because `ode_robots-config` was returning installation paths (`/home/runner/lpzrobots/include`) instead of source paths, even when using `--srcprefix` argument.

## Root Cause
The LPZRobots config scripts (selforg-config, ode_robots-config) are generated from M4 templates at configuration time. The key insight:

1. **DEVEL vs USER Mode**: The mode is baked into the generated script during M4 processing
2. **Not Runtime Switchable**: The `--srcprefix` argument updates the path variable but doesn't change the mode
3. **DEVORUSER Macro**: In the M4 template, `DEVORUSER($1,$2)` expands to `$1` in DEVEL mode or `$2` in USER mode

Example from ode_robots-config.m4:
```m4
--cflags)
  echo $CPPFLAGS DEVORUSER(-I"$srcprefix/include",-I"$prefix/include") ...
```

## Discovery
Checking the generated ode_robots-config revealed:
```bash
type=USER  # Should be DEVEL for CI
```

This explained why it was using installation paths despite our wrapper scripts.

## Solution
Instead of wrapper scripts that pass `--srcprefix`, we regenerate the config scripts in DEVEL mode:

```bash
# Check current mode
ODE_ROBOTS_TYPE=$($PROJECT_ROOT/ode_robots/ode_robots-config --type)

if [ "$ODE_ROBOTS_TYPE" != "DEVEL" ]; then
    # Regenerate from M4 template with DEVEL mode
    cd $PROJECT_ROOT/ode_robots
    rm -f ode_robots-config
    m4 -DPREFIX="$PREFIX" \
       -DSRCPREFIX="$PROJECT_ROOT/ode_robots" \
       -DDEVEL \
       -DLINUX \
       -DVERSION="1.0" \
       ode_robots-config.m4 > ode_robots-config
    chmod +x ode_robots-config
fi
```

## Why fix-ci-paths.sh Failed
The `fix-ci-paths.sh` script calls configure with `--type=DEVEL`, but the configure script may not properly pass this to M4. The direct M4 invocation ensures DEVEL mode is set.

## Benefits
1. **Correct Paths**: DEVEL mode uses source paths exclusively
2. **No Wrappers Needed**: The config scripts work correctly on their own
3. **Simpler CI**: No complex wrapper script logic

## Verification
After regeneration, config scripts should output source paths:
```bash
ode_robots-config --cflags
# Should show: -I/home/runner/work/lpz/lpz/ode_robots/include
# Not: -I/home/runner/lpzrobots/include
```