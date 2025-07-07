# macOS ARM64 Porting Guide

## Overview
Critical fixes and considerations for porting LpzRobots to Apple Silicon (M1/M2/M3) architecture.

## 1. Architecture Detection and Flags

### Makefile.conf Generation
```makefile
# Detect ARM64 architecture
ARCH := $(shell uname -m)
ifeq ($(ARCH),arm64)
    CFLAGS += -arch arm64
    CXXFLAGS += -arch arm64
    LDFLAGS += -arch arm64
endif
```

### Universal Binary Support (Optional)
```makefile
# For universal binaries
CFLAGS += -arch arm64 -arch x86_64
LDFLAGS += -arch arm64 -arch x86_64
```

## 2. SIMD Optimization Migration

### NEON Support for ARM64
```cpp
// File: selforg/matrix/matrix_neon.h
#ifdef __ARM_NEON
#include <arm_neon.h>

inline void matrix_mult_neon(const float* a, const float* b, float* c, 
                            int m, int n, int k) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j += 4) {
            float32x4_t sum = vdupq_n_f32(0.0f);
            for (int l = 0; l < k; l++) {
                float32x4_t a_vec = vdupq_n_f32(a[i * k + l]);
                float32x4_t b_vec = vld1q_f32(&b[l * n + j]);
                sum = vmlaq_f32(sum, a_vec, b_vec);
            }
            vst1q_f32(&c[i * n + j], sum);
        }
    }
}
#endif
```

### Platform-Specific Vectorization
```cpp
// Cross-platform SIMD abstraction
#if defined(__ARM_NEON)
    #define SIMD_AVAILABLE
    using simd_float4 = float32x4_t;
    #define simd_load(ptr) vld1q_f32(ptr)
    #define simd_store(ptr, val) vst1q_f32(ptr, val)
    #define simd_mul(a, b) vmulq_f32(a, b)
    #define simd_add(a, b) vaddq_f32(a, b)
#elif defined(__SSE__)
    #define SIMD_AVAILABLE
    using simd_float4 = __m128;
    #define simd_load(ptr) _mm_load_ps(ptr)
    #define simd_store(ptr, val) _mm_store_ps(ptr, val)
    #define simd_mul(a, b) _mm_mul_ps(a, b)
    #define simd_add(a, b) _mm_add_ps(a, b)
#endif
```

## 3. Framework and Library Paths

### Homebrew on ARM64
```bash
# ARM64 Homebrew location
/opt/homebrew/

# Legacy Intel Homebrew location
/usr/local/

# Detection in configure scripts
if [ -d "/opt/homebrew" ]; then
    BREW_PREFIX="/opt/homebrew"
else
    BREW_PREFIX="/usr/local"
fi
```

### Framework Search Paths
```makefile
# macOS SDK paths
MACOS_SDK = $(shell xcrun --show-sdk-path)
FRAMEWORK_SEARCH_PATHS = -F$(MACOS_SDK)/System/Library/Frameworks

# Avoid hardcoded paths
# Bad:  -I/System/Library/Frameworks/OpenGL.framework/Headers
# Good: -I$(MACOS_SDK)/System/Library/Frameworks/OpenGL.framework/Headers
```

## 4. OpenGL Deprecation Handling

### Metal Performance Shaders Alternative
```cpp
#ifdef __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC && !TARGET_OS_IPHONE
        // macOS-specific code
        #warning "OpenGL is deprecated on macOS. Consider Metal backend."
    #endif
#endif
```

### Runtime Flags for Compatibility
```bash
# Suppress OpenGL deprecation warnings during development
export OBJC_DISABLE_INITIALIZE_FORK_SAFETY=YES
./start -noshadow  # Disable shadows to avoid some OpenGL issues
```

## 5. Code Signing and Notarization

### Development Signing
```bash
# Sign binaries for local development
codesign --force --deep --sign - ./start

# Verify signature
codesign --verify --verbose ./start
```

### Entitlements for Debugging
```xml
<!-- entitlements.plist -->
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" 
          "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>com.apple.security.get-task-allow</key>
    <true/>
    <key>com.apple.security.cs.disable-library-validation</key>
    <true/>
</dict>
</plist>
```

## 6. Performance Optimizations

### ARM64-Specific Compiler Flags
```makefile
# Optimize for Apple Silicon
CXXFLAGS_ARM64 = -O3 -march=armv8.5-a+crypto+sha3 -mtune=apple-m1

# Enable Link Time Optimization
LDFLAGS += -flto=thin

# Profile-guided optimization
CXXFLAGS_PGO = -fprofile-generate
LDFLAGS_PGO = -fprofile-generate
```

### Memory Alignment
```cpp
// Ensure proper alignment for NEON
alignas(16) float matrix_data[SIZE];

// Or use aligned allocation
float* data = static_cast<float*>(std::aligned_alloc(16, size * sizeof(float)));
```

## 7. Debugging Tools

### Platform-Specific Tools
```bash
# Verify architecture
file ./start
# Output: ./start: Mach-O 64-bit executable arm64

# List dynamic dependencies
otool -L ./start

# Check for Intel translation
sysctl sysctl.proc_translated
# 0 = native ARM64, 1 = running under Rosetta 2

# Profile with Instruments
instruments -t "Time Profiler" ./start
```

### Address Sanitizer on ARM64
```makefile
# Enable sanitizers
SANITIZER_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer
CXXFLAGS += $(SANITIZER_FLAGS)
LDFLAGS += $(SANITIZER_FLAGS)

# Runtime options
export ASAN_OPTIONS=detect_leaks=1:color=always
export UBSAN_OPTIONS=print_stacktrace=1
```

## 8. Common Issues and Solutions

### Issue: Rosetta 2 Performance
```bash
# Force native ARM64 execution
arch -arm64 ./start

# Check if running native
ps -p $$ | grep -q arm64 && echo "Native ARM64" || echo "Rosetta 2"
```

### Issue: Library Architecture Mismatch
```bash
# Verify library architectures
for lib in *.dylib; do
    echo "$lib: $(lipo -info "$lib" 2>/dev/null | awk '{print $NF}')"
done

# Create universal binary from separate architectures
lipo -create libfoo_x86_64.dylib libfoo_arm64.dylib -output libfoo.dylib
```

### Issue: Missing ARM64 Dependencies
```bash
# Reinstall Homebrew packages for ARM64
brew reinstall --build-from-source <package>

# Or use prebuilt bottles
brew install --force-bottle <package>
```

## Testing Matrix

| Component | x86_64 Status | ARM64 Status | Notes |
|-----------|--------------|--------------|-------|
| selforg | ✓ | ✓ | NEON optimizations added |
| ode_robots | ✓ | ✓ | Full compatibility |
| opende | ✓ | ✓ | Minor warnings only |
| guilogger | ✓ | ✓ | Qt6 required |
| matrixviz | ✓ | ✓ | Qt6 required |
| ga_tools | ✓ | Pending | Needs C++17 fixes |
| configurator | ✓ | Pending | Qt6 migration needed |

## Performance Benchmarks

```bash
# Simple matrix benchmark
time ode_robots/simulations/template_sphererobot/start -noshadow -r 10000

# Results (example):
# Intel i9:     12.3s
# M1 (Rosetta): 15.2s  
# M1 (Native):   8.7s  <- 40% faster than Intel
```