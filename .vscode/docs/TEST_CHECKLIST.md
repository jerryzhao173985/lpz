# VSCode C++ Configuration Test Checklist

## Pre-Migration Testing (Current Microsoft C++ Setup)

### Test Environment
- **Date**: 2025-01-28
- **VSCode Version**: [Record your version]
- **C++ Extension Version**: [Record version]
- **System**: macOS M4 Max, 64GB RAM

### 1. Code Formatting Tests

#### Test 1.1: Manual Format
- [ ] Open: `selforg/matrix/matrix.cpp`
- [ ] Make the code messy (wrong indentation)
- [ ] Press: `Shift+Alt+F`
- [ ] **Expected**: Code reformats according to .clang-format
- [ ] **Actual Result**: ___________________
- [ ] **Time taken**: ___________________

#### Test 1.2: Format on Save
- [ ] Open: `selforg/controller/sox.cpp`
- [ ] Add extra spaces/tabs
- [ ] Save: `Cmd+S`
- [ ] **Expected**: Auto-formats on save
- [ ] **Actual Result**: ___________________

#### Test 1.3: Include Sorting
- [ ] Open any .cpp file with multiple includes
- [ ] Disorder the includes
- [ ] Format the file
- [ ] **Expected**: Includes sorted per .clang-format rules
- [ ] **Actual Result**: ___________________

### 2. IntelliSense Performance Tests

#### Test 2.1: Code Completion Speed
- [ ] Open: `selforg/matrix/matrix.h`
- [ ] Type: `Matrix m; m.` (note the dot)
- [ ] **Measure time** until suggestions appear
- [ ] **Time**: ___________________
- [ ] **Completions shown**: ___________________

#### Test 2.2: Template Completion
- [ ] Open: `selforg/controller/controllerbase.h`
- [ ] In a method, type: `std::vector<Matrix> v; v.`
- [ ] **Measure time** until suggestions appear
- [ ] **Time**: ___________________
- [ ] **Quality of suggestions**: ___________________

#### Test 2.3: NEON Intrinsics
- [ ] Open: `selforg/matrix/matrix_neon.h`
- [ ] Type: `float32x4_t v = vld` (NEON load intrinsic)
- [ ] **Expected**: Shows NEON intrinsic completions
- [ ] **Actual Result**: ___________________
- [ ] **Accuracy**: ___________________

### 3. Code Navigation Tests

#### Test 3.1: Go to Definition (F12)
- [ ] Open: `ode_robots/robots/sphererobot3masses.cpp`
- [ ] Find usage of `OdeRobot`
- [ ] Press F12 on it
- [ ] **Measure time** to jump to definition
- [ ] **Time**: ___________________
- [ ] **Correct location?**: ___________________

#### Test 3.2: Find All References (Shift+F12)
- [ ] Open: `selforg/abstractcontroller.h`
- [ ] Click on `step` method
- [ ] Press Shift+F12
- [ ] **Measure time** to show all references
- [ ] **Time**: ___________________
- [ ] **Number of references found**: ___________________
- [ ] **Accuracy**: ___________________

#### Test 3.3: Peek Definition (Alt+F12)
- [ ] Open any simulation file
- [ ] Find a robot class usage
- [ ] Press Alt+F12
- [ ] **Time to show peek**: ___________________
- [ ] **Shows correct definition?**: ___________________

### 4. Error Detection Tests

#### Test 4.1: Syntax Errors
- [ ] Open: `selforg/controller/sox.cpp`
- [ ] Add syntax error: `int x = ` (incomplete)
- [ ] **Time to show red squiggle**: ___________________
- [ ] **Error message quality**: ___________________

#### Test 4.2: Type Errors
- [ ] In any cpp file, write: `Matrix m = "string";`
- [ ] **Time to detect**: ___________________
- [ ] **Error clarity**: ___________________

#### Test 4.3: Include Errors
- [ ] Add: `#include <nonexistent.h>`
- [ ] **Detection time**: ___________________
- [ ] **Suggestion quality**: ___________________

### 5. Large File Performance

#### Test 5.1: Open Large File
- [ ] Open: `ode_robots/osg/primitive.cpp` (or another large file)
- [ ] **Time to syntax highlight**: ___________________
- [ ] **Time to IntelliSense ready**: ___________________
- [ ] **Memory usage increase**: ___________________

### 6. Project-Wide Operations

#### Test 6.1: Rename Symbol (F2)
- [ ] Open a header file
- [ ] Select a method name
- [ ] Press F2 and rename
- [ ] **Time to find all occurrences**: ___________________
- [ ] **Number of files affected**: ___________________
- [ ] **Success?**: ___________________

#### Test 6.2: Workspace Symbol Search (Cmd+T)
- [ ] Press Cmd+T
- [ ] Type: "Matrix"
- [ ] **Time to show results**: ___________________
- [ ] **Result quality**: ___________________

### 7. Memory and CPU Monitoring

#### Before Testing
- [ ] Note VSCode memory usage: ___________________
- [ ] Note C++ extension memory: ___________________

#### After All Tests
- [ ] Final memory usage: ___________________
- [ ] Peak memory during tests: ___________________
- [ ] CPU usage during navigation: ___________________

### 8. Debug Configuration Test

#### Test 8.1: Set Breakpoint
- [ ] Open main.cpp of any simulation
- [ ] Set breakpoint (click line number)
- [ ] **Visual feedback time**: ___________________

#### Test 8.2: Debug Hover
- [ ] Hover over a variable
- [ ] **Type information shown?**: ___________________
- [ ] **Time to show**: ___________________

### Performance Baseline Summary

| Operation | Time | Quality (1-10) | Notes |
|-----------|------|----------------|-------|
| Format file | ___ | ___ | |
| Code completion | ___ | ___ | |
| Go to definition | ___ | ___ | |
| Find references | ___ | ___ | |
| Error detection | ___ | ___ | |
| Symbol rename | ___ | ___ | |
| Initial indexing | ___ | N/A | |
| Memory usage | ___ | N/A | |

### Known Issues with Current Setup
1. ___________________
2. ___________________
3. ___________________

### Features Working Well
1. ___________________
2. ___________________
3. ___________________

---

## Ready for Migration?

- [ ] All critical features tested
- [ ] Baseline metrics documented
- [ ] No blocking issues found
- [ ] Backup verified

If all boxes checked, proceed with:
```bash
.vscode/scripts/toggle-language-server.sh
```