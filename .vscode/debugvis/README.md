# 🔍 LPZRobots Debug Visualizers

This directory contains custom debug visualizers for the LPZRobots project, making it easier to inspect complex data structures during debugging sessions.

## 🎯 Features

### Supported Types

1. **matrix::Matrix**
   - Shows dimensions and data in a grid format
   - Small matrices (≤16 elements): Full content display
   - Large matrices: Summary statistics with corner preview
   - Expandable rows for detailed inspection

2. **OdeRobot**
   - Displays robot name, sensor/motor counts
   - Shows current sensor and motor values
   - Position information when available

3. **AbstractController & derived classes**
   - Shows controller type, sensor/motor configuration
   - Displays time step counter
   - Expandable view of internal matrices (A, C, S, h, etc.)

4. **Pos** (3D position)
   - Compact (x, y, z) display

5. **Color**
   - RGBA values with 2 decimal precision

## 🚀 Usage

The debug visualizers are automatically loaded when you start a debugging session in VSCode. They work with both LLDB (macOS) and GDB (Linux).

### In the Debug Console

```lldb
# LLDB (macOS)
(lldb) p myMatrix
Matrix[3x3] = [1 0 0; 0 1 0; 0 0 1]

# GDB (Linux)  
(gdb) p myMatrix
Matrix[3x3]:
  [ 1.0  0.0  0.0 ]
  [ 0.0  1.0  0.0 ]
  [ 0.0  0.0  1.0 ]
```

### Custom Commands

#### Matrix inspection (GDB)
```gdb
(gdb) matrix A
Matrix[3 x 3]:
  [ 0.5  0.1  0.0 ]
  [ 0.1  0.8  0.2 ]
  [ 0.0  0.2  0.6 ]
```

#### Robot state (GDB)
```gdb
(gdb) robot myRobot
Robot: Sphererobot3Masses
  Sensors: 3
  Motors: 3
  Sensor values: [ 0.125 -0.234 0.567 ]
```

### In the Variables View

When debugging, the Variables view will show enhanced representations:
- Matrices show dimensions and preview
- Robots show sensor/motor counts
- Controllers show their type and state

Expand variables to see:
- Individual matrix rows
- Sensor/motor arrays
- Controller parameters

## 🔧 Implementation Details

### File Structure
```
.vscode/debugvis/
├── README.md                # This file
├── lpzrobots_debugvis.py   # GDB pretty printers
├── lpzrobots_lldb.py       # LLDB formatters
├── .gdbinit                # GDB initialization
└── .lldbinit               # LLDB initialization
```

### How It Works

1. **GDB (Linux)**
   - Uses Python pretty printers
   - Loaded via `.gdbinit` on debug start
   - Provides custom `to_string()` and `children()` methods

2. **LLDB (macOS)**
   - Uses Python summary and synthetic providers
   - Loaded via setup commands in `launch.json`
   - Type summaries for compact display
   - Synthetic children for expandable views

## 🎨 Customization

### Adding New Types

To add visualization for a new type:

1. **For GDB** - Edit `lpzrobots_debugvis.py`:
```python
class MyTypePrinter:
    def __init__(self, val):
        self.val = val
        
    def to_string(self):
        return f"MyType: {self.val['field']}"
        
    def children(self):
        return [('field', self.val['field'])]
```

2. **For LLDB** - Edit `lpzrobots_lldb.py`:
```python
def mytype_summary(valobj, internal_dict):
    return f"MyType: {valobj.GetChildMemberWithName('field').GetSummary()}"
    
# Register in __lldb_init_module
debugger.HandleCommand('type summary add -F lpzrobots_lldb.mytype_summary MyType')
```

### Modifying Display Format

- Matrix precision: Change `:.4g` format specifiers
- Array limits: Modify the `10` limit in array displays
- Summary length: Adjust truncation limits

## 🐛 Troubleshooting

### Visualizers Not Loading

1. Check debug console for error messages
2. Verify Python is available:
   ```
   (lldb) script import sys; print(sys.version)
   (gdb) python print(sys.version)
   ```
3. Manually load visualizers:
   ```
   (lldb) command script import /path/to/.vscode/debugvis/lpzrobots_lldb.py
   (gdb) source /path/to/.vscode/debugvis/.gdbinit
   ```

### Type Not Recognized

1. Check exact type name with:
   ```
   (lldb) frame variable -T myVar
   (gdb) ptype myVar
   ```
2. Update type patterns in visualizer registration

### Performance Issues

For very large matrices:
- Limit preview size in `_format_summary_matrix()`
- Reduce `num_children()` return value
- Disable expansion for huge arrays

## 📚 References

- [GDB Pretty Printing](https://sourceware.org/gdb/current/onlinedocs/gdb/Pretty-Printing.html)
- [LLDB Data Formatters](https://lldb.llvm.org/use/variable.html)
- [VSCode C++ Debugging](https://code.visualstudio.com/docs/cpp/cpp-debug)

## 💡 Tips

1. Use `p/r` in LLDB to see raw values without formatting
2. Use `set print pretty off` in GDB to disable formatting temporarily
3. Combine with watch expressions for live updates
4. Use conditional breakpoints with visualizer data

Happy debugging! 🎉