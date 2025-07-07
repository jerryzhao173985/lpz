# Documentation Reorganization Plan

## Overview
Consolidating and organizing 190+ .md files scattered throughout the LPZRobots repository.

## Directory Structure

```
documentation/
├── 01_project_docs/      # Project overview, status, journey
├── 02_build_docs/        # Build system, CMake, compilation
├── 03_migration_docs/    # C++17, modernization, porting
├── 04_component_docs/    # Component-specific documentation
├── 05_testing_docs/      # Testing, quality, validation
├── 06_algorithm_docs/    # Controllers, algorithms, theory
├── 07_analysis_docs/     # Code analysis, architecture, design
├── 08_guides/            # How-to guides, tutorials, quick starts
└── archive/              # Outdated/superseded documentation
```

## Categorization Rules

### 01_project_docs/
- Project status files (PROJECT_*, FINAL_*)
- Journey/summary files (*_COMPLETE.md, *_SUMMARY.md)
- Achievement reports
- Roadmaps and plans

### 02_build_docs/
- CMAKE_* files
- BUILD_* files
- Makefile documentation
- Platform-specific build docs
- VSCode integration

### 03_migration_docs/
- C++17/20 modernization
- macOS ARM64 porting
- API migration guides
- Refactoring documentation

### 04_component_docs/
- selforg/ specific docs
- ode_robots/ specific docs
- Component READMEs
- Module documentation

### 05_testing_docs/
- Test documentation
- Quality reports
- Coverage analysis
- BDD/TDD docs

### 06_algorithm_docs/
- Controller documentation
- Algorithm implementations
- Mathematical foundations
- Novel algorithms

### 07_analysis_docs/
- Architecture analysis
- Dependency analysis
- Code quality reports
- Design documentation

### 08_guides/
- Quick start guides
- Installation guides
- User guides
- Developer guides

## Consolidation Strategy

1. **Identify Redundant Files**: Many CMAKE_* and BUILD_* files contain overlapping content
2. **Merge Similar Content**: Combine related documents into comprehensive guides
3. **Update Outdated Content**: Ensure all docs reflect current state
4. **Create Navigation**: Add README.md with clear structure
5. **Archive Old Versions**: Move superseded docs to archive/

## Special Files to Preserve
- CLAUDE.md (AI instructions - keep in root)
- README.md (main project README - keep in root)
- LICENSE (keep in root)
- Component-specific READMEs (keep in place)

## Next Steps
1. Scan and categorize all .md files
2. Identify duplicates and outdated content
3. Consolidate similar documents
4. Move files to appropriate directories
5. Update cross-references
6. Create master index