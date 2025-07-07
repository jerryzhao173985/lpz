# Contributing to LPZRobots

Thank you for your interest in contributing to LPZRobots! This document provides guidelines and information for contributors.

## Table of Contents
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Code Style](#code-style)
- [Making Changes](#making-changes)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Documentation](#documentation)

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/lpzrobots.git
   cd lpzrobots
   ```
3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/georgmartius/lpzrobots.git
   ```

4. **Set up pre-commit hooks** (recommended):
   ```bash
   pip install pre-commit
   pre-commit install
   ```
   This will automatically check your code before each commit.

## Development Setup

### Prerequisites
- C++17 compatible compiler (clang++ 14+ or g++ 11+)
- Qt6 for GUI tools
- OpenSceneGraph 3.6+
- GSL (GNU Scientific Library)

### macOS Setup
```bash
# Install dependencies via Homebrew
brew install qt@6 open-scene-graph gsl readline gnuplot

# Configure and build
make conf
make all -j8
```

### Linux Setup
```bash
# Ubuntu/Debian
sudo apt-get install qt6-base-dev libopenscenegraph-dev \
    libgsl-dev libreadline-dev libncurses5-dev \
    libglu1-mesa-dev freeglut3-dev

# Build
make conf
make all -j8
```

## Code Style

### C++ Standards
- Use C++17 features where appropriate
- Prefer `nullptr` over `NULL`
- Use `override` for virtual functions
- Use `noexcept` instead of `throw()`
- Prefer `using` over `typedef`
- Use smart pointers for memory management

### Formatting
```cpp
// Use 2-space indentation
class MyClass : public BaseClass {
public:
  MyClass() = default;
  virtual ~MyClass() = default;
  
  void myMethod() override {
    if (condition) {
      doSomething();
    }
  }
  
private:
  int m_member = 0;
};
```

### Naming Conventions
- Classes: `PascalCase`
- Methods: `camelCase`
- Variables: `camelCase`
- Constants: `UPPER_SNAKE_CASE`
- Member variables: `m_` prefix or trailing `_`

## Making Changes

### Workflow
1. **Create a feature branch**:
   ```bash
   git checkout -b feature/my-feature
   ```

2. **Make your changes**:
   - Keep commits focused and atomic
   - Write clear commit messages
   - Follow the existing code style

3. **Test your changes**:
   ```bash
   # Run tests
   make test
   
   # Test with sanitizers
   make asan
   ./start_asan -noshadow
   ```

### Commit Messages
Follow this format:
```
component: Brief description (max 50 chars)

Longer description explaining:
- What changed
- Why it changed
- Any important details

Fixes #123
```

Example:
```
selforg: Fix memory leak in Matrix class

The Matrix copy constructor was not properly handling
the reference count, leading to memory leaks when
matrices were copied in tight loops.

Added proper reference counting and RAII cleanup.

Fixes #456
```

## Testing

### Running Tests
```bash
# Build and run all tests
make test

# Run specific component tests
cd selforg/tests
make
./run_tests

# Test with sanitizers
make asan   # AddressSanitizer
make tsan   # ThreadSanitizer
make ubsan  # UndefinedBehaviorSanitizer
```

### Writing Tests
- Add tests for new functionality
- Use the doctest framework for unit tests
- Place tests in the appropriate `tests/` directory
- Follow existing test patterns

Example test:
```cpp
TEST_CASE("Matrix multiplication") {
  Matrix a(2, 2);
  Matrix b(2, 2);
  
  a.val(0,0) = 1; a.val(0,1) = 2;
  a.val(1,0) = 3; a.val(1,1) = 4;
  
  b.toId(); // Identity matrix
  
  Matrix c = a * b;
  
  CHECK(c == a);
}
```

## Submitting Changes

### Pull Request Process

1. **Update your branch**:
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Push to your fork**:
   ```bash
   git push origin feature/my-feature
   ```

3. **Create Pull Request**:
   - Go to GitHub and create a PR from your branch
   - Fill out the PR template
   - Reference any related issues

### PR Guidelines
- Keep PRs focused on a single feature/fix
- Include tests for new functionality
- Update documentation as needed
- Ensure all tests pass
- Respond to review feedback promptly

### Code Review
- All PRs require review before merging
- Address reviewer comments
- Be open to feedback and suggestions
- Thank reviewers for their time

## Documentation

### Code Documentation
- Use Doxygen-style comments for public APIs:
  ```cpp
  /**
   * @brief Calculate the pseudoinverse of the matrix
   * @param tolerance Singular value threshold
   * @return Pseudoinverse matrix
   */
  Matrix pseudoInverse(double tolerance = 1e-10) const;
  ```

### Updating Documentation
- Keep README.md files current
- Update relevant docs in `docs/` directory
- Add examples for new features
- Document breaking changes

### Documentation Structure
```
docs/
├── README.md           # Documentation index
├── current/           # Current status
├── guides/            # User guides
├── reference/         # API reference
└── migration/         # Migration guides
```

## Component-Specific Guidelines

### selforg (Controllers)
- Follow homeokinetic principles
- Maintain numerical stability
- Document algorithm parameters
- Provide usage examples

### ode_robots (Physics)
- Ensure physical plausibility
- Test with different timesteps
- Document robot parameters
- Handle edge cases gracefully

### GUI Tools
- Follow Qt6 best practices
- Ensure cross-platform compatibility
- Test on high-DPI displays
- Maintain keyboard shortcuts

## Getting Help

- **Questions**: Open a GitHub Discussion
- **Bugs**: Open a GitHub Issue
- **Security**: Email maintainers directly

## Recognition

Contributors will be recognized in:
- The project README
- Release notes
- Academic publications (where appropriate)

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (see LICENSE file).

---

Thank you for contributing to LPZRobots! Your efforts help advance research in self-organizing robotics.