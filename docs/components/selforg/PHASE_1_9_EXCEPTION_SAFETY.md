# Phase 1.9: Exception Safety Implementation Plan

## Current State
The Matrix class uses raw pointers with malloc/free:
```cpp
private:
  I m, n;        // dimensions
  I buffersize;  // allocated size (can be > m*n for reuse)
  D* data;       // raw pointer to data
```

## Simple Exception Safety Solution

### Option 1: Minimal Change - Smart Pointer (REJECTED)
Using std::unique_ptr with custom deleter would require changing the data member type, which could break binary compatibility.

### Option 2: RAII Helper Pattern (CHOSEN) ✓
Keep the existing structure but use RAII helpers during operations that might throw.

## Implementation Details

### Step 1: Create a Simple RAII Guard
```cpp
// Add to matrix.cpp (not in header - internal use only)
namespace {
  struct MatrixDataGuard {
    D* ptr;
    explicit MatrixDataGuard(size_t size) : ptr(nullptr) {
      if (size > 0) {
        ptr = static_cast<D*>(malloc(sizeof(D) * size));
        if (!ptr) throw std::bad_alloc();
      }
    }
    ~MatrixDataGuard() { if (ptr) free(ptr); }
    D* release() { D* p = ptr; ptr = nullptr; return p; }
    
    // Prevent copying
    MatrixDataGuard(const MatrixDataGuard&) = delete;
    MatrixDataGuard& operator=(const MatrixDataGuard&) = delete;
  };
}
```

### Step 2: Fix Copy Constructor
Current implementation is not exception-safe. If allocation succeeds but memcpy somehow fails (unlikely but possible with signal handlers), we leak.

```cpp
// Current (NOT exception-safe):
Matrix::Matrix(const Matrix& c) : m(0), n(0), buffersize(0), data(0) {
  copy(c);  // If this throws after allocation, we leak
}

// Fixed version:
Matrix::Matrix(const Matrix& c) : m(c.m), n(c.n), buffersize(0), data(0) {
  if (m * n > 0) {
    MatrixDataGuard guard(m * n);
    memcpy(guard.ptr, c.data, m * n * sizeof(D));
    // If we reach here, no exceptions were thrown
    buffersize = m * n;
    data = guard.release();  // Transfer ownership
  }
}
```

### Step 3: Fix Assignment Operator
The copy-and-swap idiom provides strong exception guarantee:

```cpp
// Add swap method first
void Matrix::swap(Matrix& other) noexcept {
  std::swap(m, other.m);
  std::swap(n, other.n);
  std::swap(buffersize, other.buffersize);
  std::swap(data, other.data);
}

// Then use it for assignment
Matrix& Matrix::operator=(const Matrix& c) {
  if (this != &c) {
    Matrix temp(c);  // If this throws, *this is unchanged
    swap(temp);      // noexcept - cannot fail
    // temp's destructor cleans up our old data
  }
  return *this;
}
```

### Step 4: Fix Other Potentially Throwing Operations

#### Matrix multiplication result allocation:
```cpp
// In matrix.cpp - operations that create new matrices
Matrix Matrix::operator*(const Matrix& fac) const {
  assert(n == fac.m);
  // Use RAII for result allocation
  Matrix result(m, fac.n);  // If this throws, no leak
  mult(*this, fac, result);
  return result;  // NRVO or move
}
```

#### The set() method:
```cpp
void Matrix::set(I _m, I _n, const D* _data) {
  if (_m * _n > buffersize) {
    // Need reallocation - use RAII
    MatrixDataGuard guard(_m * _n);
    
    // Only update members after successful allocation
    if (data) free(data);
    data = guard.release();
    buffersize = _m * _n;
  }
  
  m = _m;
  n = _n;
  
  if (_data) {
    memcpy(data, _data, m * n * sizeof(D));
  } else {
    memset(data, 0, m * n * sizeof(D));
  }
}
```

### Step 5: Add noexcept Specifications
Mark operations that cannot throw:

```cpp
// In matrix.h
class Matrix {
  // These never throw:
  [[nodiscard]] I getM() const noexcept { return m; }
  [[nodiscard]] I getN() const noexcept { return n; }
  [[nodiscard]] I size() const noexcept { return n * m; }
  
  // Move operations should be noexcept
  Matrix(Matrix&& c) noexcept;
  Matrix& operator=(Matrix&& c) noexcept;
  
  // Add swap
  void swap(Matrix& other) noexcept;
};
```

## Testing Strategy

### 1. Exception Safety Test
```cpp
void testExceptionSafety() {
  // Test 1: Copy constructor with allocation failure
  {
    Matrix large(10000, 10000);  // 800MB
    
    // Simulate low memory by allocating most of it
    std::vector<std::unique_ptr<char[]>> memory_hog;
    try {
      while (true) {
        memory_hog.push_back(std::make_unique<char[]>(100'000'000));
      }
    } catch (...) {}
    
    // Now try to copy - should throw bad_alloc
    try {
      Matrix copy(large);
      assert(false && "Should have thrown");
    } catch (const std::bad_alloc&) {
      // Expected - check no leak with valgrind/asan
    }
  }
  
  // Test 2: Assignment with exception
  {
    Matrix a(100, 100);
    Matrix b(10000, 10000);  // Large
    
    // Fill memory
    std::vector<std::unique_ptr<char[]>> memory_hog;
    try {
      while (true) {
        memory_hog.push_back(std::make_unique<char[]>(100'000'000));
      }
    } catch (...) {}
    
    // Assignment should fail but 'a' should remain unchanged
    Matrix a_backup = a;
    try {
      a = b;  // Should throw
      assert(false && "Should have thrown");
    } catch (const std::bad_alloc&) {
      // Verify 'a' is unchanged
      assert(a.getM() == a_backup.getM());
      assert(a.getN() == a_backup.getN());
    }
  }
}
```

### 2. Leak Detection
Run all tests with AddressSanitizer:
```bash
make asan
ASAN_OPTIONS=detect_leaks=1 ./test_matrix_exceptions
```

## Implementation Time
- Step 1-2: 30 minutes (RAII guard + copy constructor)
- Step 3: 20 minutes (assignment operator)
- Step 4: 40 minutes (other operations)
- Step 5: 10 minutes (noexcept markers)
- Testing: 30 minutes

**Total: ~2 hours**

## Benefits
1. **Strong exception guarantee** for copy operations
2. **No memory leaks** even if exceptions are thrown
3. **Minimal code changes** - existing API unchanged
4. **Zero runtime overhead** in the common case
5. **Easy to verify** with existing tools (ASan, Valgrind)

## Next Steps
After implementing this, the Matrix class will be fully exception-safe and ready for more advanced optimizations in Phase 2.