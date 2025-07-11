# Code Quality Tools Configuration

This document explains the `.clang-format` and `.clang-tidy` configurations for the LPZRobots project.

## .clang-format

The formatting configuration is based on Google style with adjustments for this project:

### Key Features:
- **IndentWidth: 4** - Matches existing codebase convention
- **Standard: c++17** - Ensures proper parsing of C++17 features
- **IncludeBlocks: Regroup** - Organizes includes in a logical order:
  1. Project headers (selforg, ode_robots, etc.)
  2. Local headers
  3. Qt headers
  4. OpenSceneGraph headers
  5. Standard C headers
  6. Standard C++ headers
  7. External libraries

### Usage:
```bash
# Format a single file
clang-format -i path/to/file.cpp

# Format all files in a directory
find selforg -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Check formatting without modifying
clang-format --dry-run --Werror file.cpp

# Format only changed lines (git)
git diff -U0 --no-color HEAD^ | clang-format-diff -i -p1
```

## .clang-tidy

The configuration focuses on C++17 modernization while maintaining practical development flexibility.

### Check Categories:

1. **Modernization (modernize-*)**: Essential for C++17 migration
   - Converts to modern C++ idioms
   - Helps identify deprecated patterns
   - Suggests performance improvements

2. **Performance (performance-*)**: Critical for real-time simulation
   - Identifies unnecessary copies
   - Suggests move semantics usage
   - Finds inefficient algorithms

3. **Bug Detection (bugprone-*)**: Catches common mistakes
   - Use-after-move errors
   - Integer division issues
   - Virtual function problems

4. **Readability**: Balanced selection for code clarity
   - Simplifies boolean expressions
   - Ensures consistent style
   - Makes code more maintainable

5. **Core Guidelines**: Selective checks for safety
   - Prevents slicing
   - Ensures proper initialization
   - Virtual function best practices

### Usage:
```bash
# Run on a single file with compilation database
clang-tidy -p build/ path/to/file.cpp

# Run with specific checks
clang-tidy -checks='-*,modernize-*' file.cpp -- -std=c++17

# Apply fixes automatically
clang-tidy -fix -p build/ file.cpp

# Run on changed files only
git diff --name-only HEAD | grep -E '\.(cpp|h)$' | xargs clang-tidy -p build/

# Generate compilation database with CMake
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build

# Or with Make-based build
bear -- make
```

### Integration with CI/CD:

```bash
# Example for CI pipeline
#!/bin/bash
# Run clang-tidy on all project files
find selforg ode_robots -name "*.cpp" -o -name "*.h" | \
  parallel -j$(nproc) clang-tidy -p build/ {} \; > clang-tidy-report.txt

# Check if there are any warnings
if grep -q "warning:" clang-tidy-report.txt; then
  echo "clang-tidy found issues"
  cat clang-tidy-report.txt
  exit 1
fi
```

## Best Practices

1. **Gradual Adoption**: 
   - Start with formatting existing code
   - Fix modernization warnings incrementally
   - Focus on new code first

2. **Team Collaboration**:
   - Review the configurations with your team
   - Adjust check severity based on team experience
   - Document any suppressions

3. **Performance Considerations**:
   - Use compilation database for faster analysis
   - Run incrementally on changed files
   - Consider parallelizing for large codebases

4. **Suppressing Warnings**:
   ```cpp
   // Suppress specific warning
   // NOLINTNEXTLINE(modernize-use-auto)
   std::vector<int>::iterator it = vec.begin();
   
   // Suppress for a block
   // NOLINTBEGIN(performance-unnecessary-copy-initialization)
   auto copy = expensive_object;
   // ... code ...
   // NOLINTEND(performance-unnecessary-copy-initialization)
   ```

5. **IDE Integration**:
   - Most IDEs support .clang-format natively
   - Configure format-on-save for consistency
   - Use clangd for real-time clang-tidy feedback

## Customization

Both configurations are designed to be flexible. You can:
- Add/remove checks based on your needs
- Adjust warning levels
- Modify style preferences

The configurations strike a balance between code quality and practical development, supporting your C++17 modernization goals while not being overly restrictive.