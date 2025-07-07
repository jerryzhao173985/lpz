# GitHub Actions Workflows

## Quick Start

The main CI workflow is `simple-ci.yml` which mirrors the standard local build process documented in the main README.

## Core Workflow

### simple-ci.yml
This is the primary CI workflow that runs automatically on:
- Push to main/master branch
- Pull requests to main/master branch
- Manual trigger via workflow_dispatch

**What it does:**
1. Builds on Ubuntu 22.04 and macOS 14 (Apple Silicon)
2. Tests both Debug and Release configurations
3. Installs the exact same dependencies as documented in README
4. Builds using CMake presets (modern approach)
5. Runs tests with CTest
6. Verifies a simulation can be built
7. Also tests the legacy Make system for compatibility

**Key principle:** If it passes CI, it will work locally with the same commands.

## Additional Workflows (Advanced)
These workflows provide additional quality checks and are available for projects that need them:

- **ci.yml** - Comprehensive CI with extensive matrix testing
- **ci-comprehensive.yml** - Full integration test suite  
- **code-quality.yml** - Static analysis and linting
- **test-coverage.yml** - Code coverage reporting
- **advanced-testing.yml** - Detailed test categories
- **performance-tracking.yml** - Performance regression testing
- **nightly.yml** - Scheduled extensive testing
- **release.yml** - Automated release creation

## Enabling Workflows

1. The `simple-ci.yml` runs automatically on:
   - Push to main branch
   - Pull requests to main branch
   - Manual trigger (workflow_dispatch)

2. To enable additional workflows in your fork:
   - They are triggered by similar events
   - Some (like nightly) run on schedule
   - Review each workflow file for specific triggers

## Local Testing

Before pushing, you can test locally:

```bash
# CMake build
cmake --preset=default
cmake --build build/default

# Make build (legacy)
make conf  # Interactive configuration
make all
```

## Workflow Maintenance

- Keep `simple-ci.yml` minimal and focused
- Advanced workflows can be customized per-project needs
- Update dependencies in workflows when README changes