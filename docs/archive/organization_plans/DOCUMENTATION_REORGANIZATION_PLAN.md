# LPZRobots Documentation Reorganization Plan

## Current Situation
- **278 markdown files** scattered throughout the repository
- No clear organization or hierarchy
- Significant duplication and redundancy
- Mix of user docs, developer logs, and auto-generated content

## Proposed New Structure

```
docs/
├── README.md                          # Main documentation index
├── 00_QUICK_START/
│   ├── README.md                      # Getting started guide
│   ├── INSTALLATION.md                # From BUILD_GUIDE.md
│   ├── FIRST_SIMULATION.md            # Quick tutorial
│   └── TROUBLESHOOTING.md             # Common issues
│
├── 01_USER_GUIDE/
│   ├── README.md                      # User documentation index
│   ├── CONCEPTS.md                    # Core concepts
│   ├── SIMULATIONS/
│   │   ├── CREATING_SIMULATIONS.md
│   │   ├── RUNNING_SIMULATIONS.md
│   │   └── EXAMPLES.md
│   ├── CONTROLLERS/
│   │   ├── AVAILABLE_CONTROLLERS.md
│   │   ├── CONFIGURING_CONTROLLERS.md
│   │   └── NOVEL_ALGORITHMS.md
│   └── TOOLS/
│       ├── GUILOGGER.md
│       ├── MATRIXVIZ.md
│       └── CONFIGURATOR.md
│
├── 02_DEVELOPER_GUIDE/
│   ├── README.md                      # Developer documentation index
│   ├── ARCHITECTURE.md                # System architecture
│   ├── BUILD_SYSTEM/
│   │   ├── CMAKE_GUIDE.md            # Consolidated from multiple CMAKE*.md
│   │   ├── LEGACY_MAKE.md            # For reference
│   │   └── PLATFORM_NOTES.md         # macOS, Linux specifics
│   ├── COMPONENTS/
│   │   ├── SELFORG.md                # Core library docs
│   │   ├── ODE_ROBOTS.md
│   │   ├── GA_TOOLS.md
│   │   └── ECBROBOTS.md
│   ├── TESTING/
│   │   ├── TESTING_GUIDE.md          # Consolidated testing docs
│   │   ├── WRITING_TESTS.md
│   │   └── PERFORMANCE_TESTS.md
│   └── CONTRIBUTING.md
│
├── 03_API_REFERENCE/
│   ├── README.md                      # API documentation index
│   ├── CONTROLLERS/                   # Controller API docs
│   ├── ROBOTS/                        # Robot API docs
│   ├── MATRIX/                        # Matrix library API
│   └── UTILS/                         # Utility API docs
│
├── 04_ALGORITHMS/
│   ├── README.md                      # Algorithm documentation index
│   ├── HOMEOKINETIC_PRINCIPLES.md    # Theory and concepts
│   ├── NOVEL_CONTROLLERS/
│   │   ├── APEX_SOX.md               # From APEX_SOX_IMPLEMENTATION_SUMMARY.md
│   │   ├── AHSOX.md
│   │   ├── CASOX.md
│   │   ├── ITSOX.md
│   │   └── MSHC.md
│   └── PERFORMANCE_ANALYSIS.md
│
├── 05_MIGRATION_HISTORY/
│   ├── README.md                      # Historical documentation
│   ├── CPP17_MODERNIZATION.md        # Consolidated modernization docs
│   ├── CMAKE_MIGRATION.md            # Consolidated CMAKE journey
│   ├── TESTING_MIGRATION.md          # doctest migration
│   └── LESSONS_LEARNED.md
│
├── AI_ASSISTANCE/
│   └── CLAUDE.md                      # AI assistant guidance (keep updated)
│
└── archive/                           # Historical documents for reference
    ├── journey_logs/                  # Development journey files
    ├── interim_reports/               # Progress reports
    └── obsolete/                      # Outdated documentation
```

## Consolidation Strategy

### 1. CMAKE Documentation (30+ files → 3 files)
**Consolidate into `CMAKE_GUIDE.md`:**
- CMAKE_BUILD_SUCCESS.md
- CMAKE_MIGRATION_COMPLETE.md
- CMAKE_IMPLEMENTATION_SUMMARY.md
- CMAKE_BEST_PRACTICES.md
- CMAKE_SIMULATION_GUIDE.md

**Archive the rest** (interim reports, error logs, etc.)

### 2. Testing Documentation (31 files → 4 files)
**Consolidate into `TESTING_GUIDE.md`:**
- All DOCTEST_*.md files
- All TEST_*.md files
- BDD_TESTS_*.md files

### 3. Novel Algorithms (26 files → 6 files)
**Create focused docs for each algorithm:**
- Merge all APEX/AHSOX/CASOX/ITSOX/MSHC related files
- Keep only implementation details and usage

### 4. Build & Installation (15 files → 3 files)
**Consolidate into `INSTALLATION.md`:**
- BUILD_GUIDE.md
- BUILD_SYSTEM_COMPLETE.md
- QUICK_START.md

## Implementation Steps

### Phase 1: Create Directory Structure
```bash
mkdir -p docs/{00_QUICK_START,01_USER_GUIDE/{SIMULATIONS,CONTROLLERS,TOOLS}}
mkdir -p docs/{02_DEVELOPER_GUIDE/{BUILD_SYSTEM,COMPONENTS,TESTING}}
mkdir -p docs/{03_API_REFERENCE/{CONTROLLERS,ROBOTS,MATRIX,UTILS}}
mkdir -p docs/{04_ALGORITHMS/NOVEL_CONTROLLERS,05_MIGRATION_HISTORY}
mkdir -p docs/{AI_ASSISTANCE,archive/{journey_logs,interim_reports,obsolete}}
```

### Phase 2: Move Core Files
1. Move `README.md` → keep at root (points to docs/)
2. Move `CLAUDE.md` → `docs/AI_ASSISTANCE/CLAUDE.md`
3. Move build guides → `docs/00_QUICK_START/`
4. Move component docs → appropriate subdirectories

### Phase 3: Consolidate Redundant Files
1. Merge all CMAKE files → `docs/02_DEVELOPER_GUIDE/BUILD_SYSTEM/CMAKE_GUIDE.md`
2. Merge all testing files → `docs/02_DEVELOPER_GUIDE/TESTING/TESTING_GUIDE.md`
3. Merge algorithm files → `docs/04_ALGORITHMS/NOVEL_CONTROLLERS/`

### Phase 4: Archive Historical Content
1. Move journey logs → `docs/archive/journey_logs/`
2. Move interim reports → `docs/archive/interim_reports/`
3. Move obsolete docs → `docs/archive/obsolete/`

### Phase 5: Clean Up
1. Remove empty directories
2. Update all internal links
3. Create comprehensive index files
4. Generate cross-references

## Expected Benefits

1. **Improved Navigation**: Clear hierarchy makes finding docs easy
2. **Reduced Redundancy**: ~3:1 reduction in file count
3. **Better Maintenance**: Clear ownership and update paths
4. **User-Friendly**: Separation of user and developer docs
5. **Preserves History**: Archive maintains development journey

## Success Metrics

- Reduce 278 files to ~100 organized files
- All documentation accessible within 3 clicks from README
- Clear separation of concerns (user/developer/API/history)
- No duplicate information across files
- Comprehensive search and navigation

## Notes

- Keep auto-generated docs in `build/` (don't move)
- Update CI/CD to generate docs in new structure
- Consider adding Doxygen for API docs generation
- Add .gitignore entries for generated content