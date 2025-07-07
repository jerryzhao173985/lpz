# Critical Patterns and Gotchas for LPZRobots Modernization

## ⚠️ CRITICAL: Smart Pointer Array Access

### ✅ CORRECT - std::unique_ptr<T[]> DOES support operator[]
```cpp
std::unique_ptr<double[]> x = std::make_unique<double[]>(n);
x[0] = 1.0;  // This WORKS! unique_ptr<T[]> has operator[]
```

### Common Patterns

#### 1. Single Array Allocation
```cpp
// OLD
double* data = new double[size];
delete[] data;

// NEW - Option 1: unique_ptr (when size known at runtime)
std::unique_ptr<double[]> data = std::make_unique<double[]>(size);
// No delete needed!

// NEW - Option 2: vector (preferred for most cases)
std::vector<double> data(size);
```

#### 2. 2D Array Allocation
```cpp
// OLD - Error prone!
double** matrix = new double*[rows];
for(int i = 0; i < rows; i++) {
    matrix[i] = new double[cols];
}
// Complex cleanup needed

// NEW - Safe and simple
std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
```

#### 3. Member Variables
```cpp
class MyRobot {
    // OLD
    double* sensors;
    double* motors;
    
    // NEW
    std::unique_ptr<double[]> sensors;
    std::unique_ptr<double[]> motors;
    // or better:
    std::vector<double> sensors;
    std::vector<double> motors;
};
```

## 🚨 GOTCHAS to Avoid

### 1. Don't Mix unique_ptr with .get() carelessly
```cpp
// WRONG - Double delete!
std::unique_ptr<double[]> data = std::make_unique<double[]>(10);
double* raw = data.get();
delete[] raw;  // NO! unique_ptr will delete it

// CORRECT - Use .release() if transferring ownership
double* raw = data.release();  // Now you own it
delete[] raw;  // OK, but why?
```

### 2. Include Required Headers
```cpp
#include <memory>     // REQUIRED for std::unique_ptr
#include <vector>     // REQUIRED for std::vector
#include <algorithm>  // Often needed for std::sort, etc.
```

### 3. Virtual Destructors with Smart Pointers
```cpp
// If using virtual functions, ALWAYS use virtual destructor
class Base {
public:
    virtual void foo() = 0;
    virtual ~Base() = default;  // CRITICAL!
};
```

### 4. Array vs Single Object unique_ptr
```cpp
// Single object - use unique_ptr<T>
std::unique_ptr<MyClass> obj = std::make_unique<MyClass>();

// Array - use unique_ptr<T[]>
std::unique_ptr<double[]> arr = std::make_unique<double[]>(10);

// WRONG - Type mismatch!
std::unique_ptr<double> arr = std::make_unique<double[]>(10);  // ERROR!
```

## 📋 Checklist for Each File

When modernizing a file:

- [ ] Add `#include <memory>` if using unique_ptr
- [ ] Add `#include <vector>` if using vector  
- [ ] Replace `typedef` with `using`
- [ ] Replace `NULL` with `nullptr`
- [ ] Add `override` to virtual functions
- [ ] Add `noexcept` to non-throwing functions
- [ ] Replace `new[]`/`delete[]` with smart pointers
- [ ] Check for 2D arrays that need modernization
- [ ] Ensure virtual destructors where needed
- [ ] Remove manual delete statements

## 🔧 Build System Insights

### CMake is WORKING!
- Don't modify the working CMake files without testing
- The modular structure in cmake/ folder is intentional
- Legacy compatibility is maintained through options

### Build Commands That Work
```bash
# Standard build
mkdir build && cd build
cmake ..
make -j8

# Debug build with sanitizers
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_CXX_FLAGS="-fsanitize=address -fsanitize=undefined"
make -j8

# Optimized build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```

## 🎯 Final Reminders

1. **Test After Each Change** - Don't batch too many changes
2. **Use the Existing Matrix Class** - Don't reinvent the wheel
3. **Preserve Functionality** - This is research code in active use
4. **Document Non-Obvious Changes** - Future maintainers will thank you
5. **The Build System Works** - Focus on code modernization now

The project is 95% complete. Stay focused on memory safety and the remaining component fixes!