# GUI Tools Fixes Summary

## Issues Fixed

### 1. Guilogger Crashes and Plot Issues

**Problem**: Multiple issues with guilogger on macOS ARM64:
- Memory management bug causing crashes when opening config files
- Gnuplot geometry options not supported on macOS
- Qt terminal for gnuplot causing font warnings and hanging

**Fixes Applied**:

#### a) Memory Management Fix in IniSection::copy()
Fixed shallow copy bug that caused use-after-free errors:
```cpp
void IniSection::copy (IniSection& _section){
  // Deep copy implementation
  qDeleteAll(_section.vars);
  _section.vars.clear();
  
  foreach(IniVar* var, vars) {
    if (var) {
      IniVar* newVar = new IniVar();
      var->copy(*newVar);
      _section.vars.append(newVar);
    }
  }
}
```

#### b) Gnuplot Command Fix for macOS
Removed unsupported -geometry option on macOS:
```cpp
#if defined(__APPLE__)
  // macOS doesn't support -geometry option
  sprintf(cmd, "%s", gnuplotcmd.toLatin1().constData());
#else
  // Linux with X11 supports -geometry
  sprintf(cmd, "%s -geometry %ix%i -noraise", ...);
#endif
```

#### c) Signal Handling
Changed SIGPIPE handling to prevent crashes:
```cpp
signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE
```

### 2. Matrixviz Crash

**Problem**: Matrixviz crashed with segmentation fault when calling `glDeleteLists` in the destructor with an invalid OpenGL display list ID (0).

**Crash Details**:
- Exception: `EXC_BAD_ACCESS (SIGSEGV)` at address `0x00000000000001d8`
- Location: `TextureVisualisation::~TextureVisualisation()` calling `glDeleteLists(object, 1)`
- Issue: `object` was initialized to 0 but `glDeleteLists` was called unconditionally

**Fix**: Added null check before calling `glDeleteLists`:
```cpp
TextureVisualisation::~TextureVisualisation(){
  if(debug) cout << "TextureVisualisation Destruktor" << endl;
  if(object != 0) {
    makeCurrent();
    glDeleteLists( object, 1 );
  }
}
```

## Testing

Run the test script to verify the fixes:
```bash
/Users/jerry/lpzrobots_mac/test_gui_tools.sh
```

Or test manually:

### Test Guilogger Plots:
1. Run simulation with guilogger:
   ```bash
   cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot
   ./start -g
   ```
2. In guilogger window, click on channel checkboxes
3. Gnuplot windows should appear showing the plots

### Test Matrixviz:
1. Run simulation with matrixviz:
   ```bash
   cd /Users/jerry/lpzrobots_mac/ode_robots/simulations/template_sphererobot
   ./start -m
   ```
2. Matrixviz should display without crashing
3. Use different visualization modes without crashes

## Known Issues and Workarounds

### Gnuplot Display Issues on macOS
The Qt terminal for gnuplot has font loading delays and may hang. Current workarounds:
1. Use alternative terminals (png output, dumb terminal)
2. Consider using native macOS plotting solutions in future updates

## Additional Notes

- Gnuplot must be installed via Homebrew: `brew install gnuplot`
- OpenGL display lists (ID 0) are not valid and must be checked before deletion
- Signal handling (SIGPIPE) must be properly configured for pipe-based communication

## Environment Setup

Ensure these environment variables are set:
```bash
export PATH="/Users/jerry/lpzrobots_mac/guilogger:/Users/jerry/lpzrobots_mac/matrixviz:$PATH"
export QT_PLUGIN_PATH=/opt/homebrew/opt/qt@5/plugins
export QT_MAC_WANTS_LAYER=1
```

## Build Instructions

### Guilogger
```bash
cd /Users/jerry/lpzrobots_mac/guilogger
make clean && make -j8
```

### Matrixviz
```bash
cd /Users/jerry/lpzrobots_mac/matrixviz
rm -rf bin/matrixviz.app
make clean && make -j8
mkdir -p bin/matrixviz.app/Contents/MacOS
cp bin/matrixviz bin/matrixviz.app/Contents/MacOS/
```