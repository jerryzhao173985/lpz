# LPZRobots Documentation Logs

This directory contains organized documentation of the LPZRobots modernization journey, build system migration, and technical improvements.

## Directory Structure

### 01_PROJECT_OVERVIEW/
Overall project status and journey documentation
- `PROJECT_FINAL_STATUS_2025.md` - Current project status (98% complete)
- `LPZROBOTS_COMPLETE_JOURNEY.md` - Complete modernization journey

### 02_BUILD_SYSTEM/
CMake migration and build system documentation
- `CMAKE_MIGRATION_COMPLETE.md` - CMake migration status and details
- `CMAKE_TECHNICAL_REFERENCE.md` - Technical reference for CMake system
- `VSCODE_CMAKE_FIX_2025-01-28.md` - VSCode CMake integration fixes
- `build_system_fixes.md` - Build system fixes and improvements
- `dependency_generation_best_practices.md` - Modern dependency generation
- `dependency_generation_modernization.md` - Dependency modernization guide

### 03_TESTING/
Testing framework and doctest migration
- `DOCTEST_MIGRATION_COMPLETE.md` - Testing framework migration status
- `DOCTEST_API_COMPATIBILITY_REFERENCE.md` - API compatibility guide
- `TESTING_INFRASTRUCTURE_COMPLETE.md` - Complete testing infrastructure
- `TESTING_INFRASTRUCTURE_COMPLETE_JOURNEY.md` - Testing journey details
- `TESTING_DOCUMENTATION_GUIDE.md` - Testing documentation guide
- `TESTING_ERRORS_AND_FIXES_REFERENCE.md` - Common testing errors and fixes

### 04_ALGORITHMS/
Homeokinetic algorithms and novel controllers
- `HOMEOKINETIC_ALGORITHMS_COMPLETE.md` - Algorithm implementation status
- `HOMEOKINETIC_ALGORITHMS_JOURNEY.md` - Development journey
- `HOMEOKINETIC_TEST_COVERAGE.md` - Test coverage report

### 05_COMPONENTS/
Component-specific modernization
- `SELFORG_MIGRATION_COMPLETE.md` - Selforg library modernization

### 06_TROUBLESHOOTING/
Common issues and solutions
- `CRITICAL_PATTERNS_AND_GOTCHAS.md` - Important patterns to avoid issues
- `critical_fixes_summary.md` - Summary of critical fixes

### 07_TECHNICAL_REFERENCE/
Technical guides and references
- `cpp_cast_modernization.md` - C++ cast modernization guide
- `cpp_modernization_patterns.md` - C++ modernization patterns
- `macos_arm64_porting.md` - macOS ARM64 porting guide
- `technical_debt_resolved.md` - Resolved technical debt documentation

## Quick Links

- [Current Project Status](01_PROJECT_OVERVIEW/PROJECT_FINAL_STATUS_2025.md)
- [CMake Build Guide](02_BUILD_SYSTEM/CMAKE_MIGRATION_COMPLETE.md)
- [Testing Guide](03_TESTING/DOCTEST_MIGRATION_COMPLETE.md)
- [Troubleshooting](06_TROUBLESHOOTING/CRITICAL_PATTERNS_AND_GOTCHAS.md)

## Latest Updates

### 2025-01-28: Build System Improvements
- Fixed CircularBuffer subscript operator issue in ahsox.cpp
- Fixed MetaState member name issue in apexsox.cpp
- All tests passing, examples building correctly
- Organized documentation into clear subdirectories

### Archived Documentation
The `archived_docs/` directory contains older documentation that has been superseded by the consolidated files above. These are kept for historical reference but should not be used as primary documentation.

Key archived files include:
- Various CMAKE migration interim reports and logs
- BDD test implementation details
- Component-specific migration logs (SELFORG, etc.)
- Session summaries and intermediate status reports
- Detailed error logs and crash analyses

Total: 28 archived files preserving the complete development history.