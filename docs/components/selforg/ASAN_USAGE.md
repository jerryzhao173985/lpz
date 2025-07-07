# Using AddressSanitizer with LPZRobots selforg

## What is AddressSanitizer?

AddressSanitizer (ASan) is a memory error detector for C/C++ that can find:
- Buffer overflows (heap, stack, globals)
- Use-after-free errors
- Use-after-return errors (when enabled)
- Memory leaks
- Double-free errors

## Building with AddressSanitizer

### 1. Build the selforg library with ASan:
```bash
cd selforg/  # or wherever you have the selforg source
make clean
make asan
```

This creates `libselforg_dbg.a` in `build_asan/` with AddressSanitizer instrumentation.

### 2. Build examples with ASan:
When building examples or simulations, you need to add ASan flags:

```bash
cd examples/integration
make clean
make CPPFLAGS="-fsanitize=address -fno-omit-frame-pointer" LDFLAGS="-fsanitize=address"
```

## Running with AddressSanitizer

### Basic usage:
```bash
./integration
```

ASan will automatically report any memory errors it finds.

### Enable leak detection (macOS):
```bash
ASAN_OPTIONS=detect_leaks=1 ./integration
```

### Common ASan options:
```bash
# Full leak checking at exit
ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1 ./integration

# Continue after first error
ASAN_OPTIONS=halt_on_error=0 ./integration

# Increase stack trace depth
ASAN_OPTIONS=print_stack_trace=1:stack_trace_depth=30 ./integration
```

## Understanding ASan Output

When ASan detects an error, it prints:
1. Error type (e.g., "heap-buffer-overflow")
2. Where the error occurred (stack trace)
3. Where the memory was allocated
4. Shadow memory state

Example:
```
==12345==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x60200000eff4
READ of size 4 at 0x60200000eff4 thread T0
    #0 0x10a6b1234 in MyRobot::getSensors(double*, int)
    #1 0x10a6b5678 in Agent::step(double, double)
```

## Known Issues on macOS ARM64

1. **Leak detection**: May need to be explicitly enabled with `ASAN_OPTIONS=detect_leaks=1`
2. **Performance**: ASan adds 2-5x slowdown, normal for debugging
3. **Memory usage**: ASan uses more memory (up to 2x)

## Best Practices

1. **Development workflow**: Use ASan during development, not in production
2. **CI/CD**: Include ASan builds in your test suite
3. **Fix immediately**: Address all ASan errors before committing code

## Troubleshooting

If you get link errors about ASan symbols:
```bash
# Ensure consistent ASan usage
make clean  # Clean everything
make asan   # Rebuild with ASan
```

If ASan doesn't report leaks on macOS:
```bash
# Force leak checking
export MallocStackLogging=1
export ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1
./your_program
```