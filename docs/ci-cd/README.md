# LPZRobots CI/CD Documentation Hub

This directory contains comprehensive documentation for the LPZRobots CI/CD system, including detailed analyses, improvement plans, and technical guides.

## 📚 Documentation Overview

### Core Documentation

#### [CI/CD Comprehensive Review](CI-CD-COMPREHENSIVE-REVIEW.md)
Complete system analysis including:
- Current state assessment with metrics
- Build matrix coverage across platforms
- Quality gates and test coverage analysis
- Security assessment and recommendations
- Action plans with immediate, short-term, and long-term improvements

#### [Legacy Make Build Analysis](LEGACY-MAKE-BUILD-ANALYSIS.md)
Deep dive into the Legacy Make system:
- Architecture and component breakdown
- Header include problem analysis and fixes
- Comparison with modern CMake system
- Migration path and deprecation recommendations

#### [CI/CD Improvements Roadmap](CI-CD-IMPROVEMENTS-ROADMAP.md)
Phased improvement plan through 2025:
- Phase 1: Foundation (Q1 2025) ✅ COMPLETED
- Phase 2: Quality & Coverage (Q2 2025) 🚧 IN PROGRESS
- Phase 3: Optimization (Q3 2025) 📋 PLANNED
- Phase 4: Excellence (Q4 2025) 🔮 FUTURE
- Quick wins and resource requirements

#### [Legacy Make Fix Journey](LEGACY-MAKE-FIX-JOURNEY.md)
Complete troubleshooting journey:
- Initial PATH issues and fixes
- Header resolution problems
- Race condition investigation
- Final solution implementation

#### [Legacy Make CI Fix Summary](LEGACY-MAKE-CI-FIX-SUMMARY.md)
Comprehensive summary of all CI fixes:
- Issues encountered and solutions
- Build system understanding
- Current status and next steps
- Alternative approaches

## 🚀 Quick Start

### Current CI/CD Workflows

#### Simple CI (`simple-ci.yml`)
**Trigger:** Push to main/master/develop, Pull Requests
- Multi-platform builds (Ubuntu 24.04, macOS 15)
- Unit tests with sanitizers and coverage
- Performance benchmarks
- Legacy Make build (optional)

#### Code Quality (`code-quality.yml`)
**Trigger:** All pushes and PRs
- Code formatting checks (clang-format)
- Static analysis (clang-tidy, cppcheck)
- Complexity metrics (pmccabe, lizard)

#### Performance (`performance.yml`)
**Trigger:** Pull requests
- Matrix operation benchmarks
- Regression detection vs baseline
- Automated PR comments

## 📊 Current Status

| Metric | Status | Details |
|--------|--------|---------|
| Build Success | 83% | 5/6 workflows passing |
| Test Coverage | ~40% | Target: 70% |
| Platform Support | ✅ Linux/macOS | ❌ Windows |
| Build Systems | ✅ CMake | ⚠️ Legacy Make |

## 🎯 Key Achievements

### January 2025
- ✅ Fixed Legacy Make build issues in CI
- ✅ Implemented comprehensive CI/CD documentation
- ✅ Added code quality and performance tracking
- ✅ Created phased improvement roadmap
- ✅ Analyzed and documented all build system issues

### Fixed Issues
1. **Legacy Make PATH problems**: Config scripts now use relative paths correctly
2. **Header resolution**: Fixed include paths in selforg-config.m4
3. **Bad symlinks**: Cleaned up interference with header resolution
4. **Race conditions**: Addressed parallel build issues

## 🚀 Quick Start

### Running CI Locally

```bash
# Recommended: Use CMake
cmake --preset=ci
cmake --build build/ci
ctest --preset=ci

# Legacy Make (deprecated but functional)
make conf && make all

# Using GitHub Actions locally with act
brew install act  # macOS
act -j build      # Run build job
```

### Manual Workflow Dispatch

```bash
# Check CI status
gh run list --workflow=simple-ci.yml

# Trigger specific workflows
gh workflow run code-quality.yml
gh workflow run performance.yml
```

## 📈 Improvement Priorities

### Immediate (This Week)
1. Fix simulation test in Legacy Make CI
2. Add path filters to skip doc-only changes
3. Enable better dependency caching

### Short Term (This Month)
1. Increase test coverage to 60%
2. Add security scanning (CodeQL, dependency checks)
3. Optimize build times with ccache

### Long Term (This Quarter)
1. Docker-based standardized builds
2. Automated release pipeline
3. GUI testing framework

## 🛠️ Troubleshooting

### Common CI Issues

1. **Legacy Make fails with "cannot find selforg-config"**
   - Already fixed in CI scripts
   - Uses relative paths: `$DIRNAME/../selforg/selforg-config`

2. **Header file not found errors**
   - Fixed by updating include paths
   - Clean symlinks before building

3. **macOS Qt/OpenGL issues**
   - Ensure Qt6 is in PATH
   - Use `-noshadow` flag for simulations

### Debugging CI Failures

```bash
# View detailed logs
gh run view <run-id> --log

# Download artifacts
gh run download <run-id>

# Enable debug mode in workflow
env:
  ACTIONS_RUNNER_DEBUG: true
  ACTIONS_STEP_DEBUG: true
```

## 📚 Additional Resources

### Internal Documentation
- [Testing Infrastructure](../testing/) - Test framework and coverage plans
- [Build System Guide](../build/) - CMake and Make documentation
- [Developer Guide](../DEVELOPER_GUIDE.md) - Contributing guidelines

### External Resources
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [CMake Best Practices](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html)
- [CTest Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)

## 👥 Contributing

When working on CI/CD:
1. Test changes locally first using `act`
2. Create PRs to validate workflow changes
3. Update relevant documentation
4. Follow the improvement roadmap priorities

---
*Last Updated: January 2025*
*Maintainer: LPZRobots Team*