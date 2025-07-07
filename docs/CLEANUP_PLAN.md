# Documentation Cleanup Plan

---
**Document Type**: Guide  
**Last Updated**: 2025-01-06  
**Status**: Current  
**Version**: 1.0  
**Tags**: documentation, cleanup, organization  
---

## Overview

The current documentation has 286+ files with massive redundancy. This plan will consolidate to ~50 essential documents.

## Phase 1: Consolidation

### Build System (31 → 3 files)
**Keep:**
- `CMAKE_COMPREHENSIVE_REVIEW.md` (new)
- `BUILD_SYSTEM_GUIDE_CONSOLIDATED.md`
- `TROUBLESHOOTING_PLATFORM_GUIDE_CONSOLIDATED.md`

**Archive/Delete:** All others including multiple status, complete, and migration files

### Migration/Modernization (26 → 2 files)
**Keep:**
- `CONSOLIDATED_MIGRATION_SUMMARY.md`
- `REFACTORING_PATTERNS.md`

**Archive:** All other modernization status files

### Testing (21 → 3 files)
**Keep:**
- `TESTING_GUIDE.md`
- `TESTING_FRAMEWORK_COMPLETE.md`
- `test-reports/` directory

**Archive:** All phase-specific completion files

### Algorithms (14 → 2 files)
**Keep:**
- `NOVEL_ALGORITHMS_SUMMARY.md`
- `HOMEOKINETIC_ALGORITHMS_PERFECT.md`

**Archive:** Individual algorithm files

## Phase 2: Reorganization

### New Structure:
```
docs/
├── README.md                    # Documentation index
├── QUICK_START.md              # Getting started guide
├── PROJECT_STATUS.md           # Single source of truth
│
├── guides/                     # User guides
│   ├── installation.md
│   ├── first-simulation.md
│   ├── using-gui-tools.md
│   └── troubleshooting.md
│
├── reference/                  # Technical reference
│   ├── architecture.md
│   ├── algorithms.md
│   ├── api-overview.md
│   └── configuration.md
│
├── development/               # Developer documentation
│   ├── setup.md
│   ├── contributing.md
│   ├── code-style.md
│   ├── testing.md
│   └── design-patterns.md
│
├── build/                     # Build system docs
│   ├── cmake-guide.md
│   ├── make-legacy.md
│   └── platform-notes.md
│
└── archive/                   # Historical documents
    └── migration-journey/     # Keep for reference
```

## Phase 3: Content Consolidation

### 1. Create Master Documents:
- `PROJECT_STATUS.md` - Combines all status information
- `TECHNICAL_REFERENCE.md` - Combines architecture, design, algorithms
- `DEVELOPER_GUIDE.md` - Combines setup, contribution, patterns

### 2. Remove Redundancy:
- Merge duplicate content
- Keep most recent/comprehensive version
- Add forwarding references from old locations

### 3. Update Cross-References:
- Fix all internal links
- Create clear navigation paths
- Add "See also" sections

## Phase 4: Naming Standardization

### Rules:
1. **Directories**: lowercase-with-hyphens
2. **Guide files**: lowercase-with-hyphens.md
3. **Status files**: UPPERCASE_WITH_UNDERSCORES.md
4. **Remove suffixes**: _COMPLETE, _FINAL, _STATUS (redundant)

## Execution Order

1. Create new directory structure
2. Move and consolidate files
3. Update all cross-references
4. Delete redundant files
5. Update DOCUMENTATION_MAP.md
6. Verify all links work