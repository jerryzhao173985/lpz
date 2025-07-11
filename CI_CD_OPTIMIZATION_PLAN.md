# CI/CD Optimization Plan for LPZRobots

## Current State Analysis

### Build Times (Approximate)
- Quick Test: ~3 minutes
- Full Linux Build: ~8-10 minutes
- Full macOS Build: ~10-12 minutes
- Legacy Make Build: ~6-8 minutes
- Total Pipeline: ~20-25 minutes per commit

### Resource Usage
- CPU: Moderate (parallel builds help)
- Memory: High during linking (>2GB)
- Storage: ~500MB per build artifact
- Network: Package downloads on each run

## Optimization Strategy

### Phase 1: Immediate Improvements (1-2 weeks)

#### 1.1 Enhanced Caching
```yaml
# Current: Basic ccache
# Proposed: Multi-level caching

- name: Cache multiple layers
  uses: actions/cache@v3
  with:
    path: |
      ~/.cache/ccache
      build/
      ~/lpzrobots/     # Installation directory
      ~/.conan/        # Future: package cache
    key: ${{ runner.os }}-${{ matrix.build_type }}-${{ hashFiles('**/CMakeLists.txt') }}
```

#### 1.2 Dependency Prebuilding
Create a base Docker image with dependencies:
```dockerfile
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y \
    build-essential cmake libgsl-dev ...
# Prebuild boost, Qt6, etc.
```

#### 1.3 Parallel Job Optimization
```yaml
strategy:
  matrix:
    include:
      - component: core      # selforg + ode_robots
      - component: tools     # GUI tools
      - component: tests     # All tests
```

### Phase 2: Structural Improvements (1 month)

#### 2.1 Build System Convergence
- Eliminate Make system from CI (CMake only)
- Create CMake targets that match Make functionality
- Single source of truth for dependencies

#### 2.2 Artifact Sharing
```yaml
# Build once, test many
jobs:
  build-libs:
    outputs:
      artifact-name: ${{ steps.upload.outputs.artifact-name }}
    steps:
      - name: Build core libraries
      - name: Upload artifacts
        id: upload

  test-simulations:
    needs: build-libs
    strategy:
      matrix:
        simulation: [sphererobot, schlange, hexapod]
```

#### 2.3 Smart Test Selection
```bash
# Only run tests affected by changes
git diff --name-only HEAD^ | \
  xargs -I {} find . -name "test_*.cpp" -path "*{}*"
```

### Phase 3: Advanced Optimization (2-3 months)

#### 3.1 Distributed Builds
- Use GitHub Actions matrix for component parallel builds
- Implement distcc or similar for compilation distribution
- Consider self-hosted runners for heavy builds

#### 3.2 Incremental Builds
```cmake
# CMake incremental build support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Use ninja for better incremental builds
```

#### 3.3 Binary Caching
- Implement ccache cloud storage
- Use GitHub Packages for pre-built dependencies
- Cache CMake's FetchContent downloads

## Specific Optimizations

### 1. CMake Improvements
```cmake
# Precompiled headers
target_precompile_headers(selforg PRIVATE
    <vector>
    <string>
    <memory>
)

# Unity builds for faster compilation
set(CMAKE_UNITY_BUILD ON)
set(CMAKE_UNITY_BUILD_BATCH_SIZE 10)
```

### 2. CI Workflow Optimization
```yaml
# Only trigger expensive builds on main branch
on:
  push:
    branches: [main]
  pull_request:
    types: [opened, synchronize]
    
# Cancel in-progress builds
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

### 3. Test Optimization
```yaml
# Run quick tests first
jobs:
  quick-tests:
    timeout-minutes: 5
    # Fail fast on quick tests
    
  full-tests:
    needs: quick-tests
    if: success()
```

### 4. Platform-Specific Optimizations

#### Linux
- Use container-based builds
- Leverage GitHub's Ubuntu runners efficiently
- Consider Alpine Linux for smaller images

#### macOS
- Cache Homebrew installations
- Use Homebrew bundle for reproducible deps
- Consider macOS-specific build tools

### 5. Monitoring and Metrics

#### Build Time Tracking
```yaml
- name: Record build time
  run: |
    echo "BUILD_TIME=$((END_TIME - START_TIME))" >> $GITHUB_ENV
    
- name: Upload metrics
  run: |
    curl -X POST https://metrics.example.com/ci \
      -d "build_time=$BUILD_TIME&commit=$GITHUB_SHA"
```

#### Resource Usage
- Monitor runner resource usage
- Track cache hit rates
- Identify bottlenecks

## Implementation Roadmap

### Week 1-2
- [ ] Implement enhanced caching
- [ ] Add build time metrics
- [ ] Optimize parallel job count

### Week 3-4
- [ ] Create dependency Docker image
- [ ] Implement artifact sharing
- [ ] Add incremental build support

### Month 2
- [ ] Set up distributed compilation
- [ ] Implement smart test selection
- [ ] Create performance dashboard

### Month 3
- [ ] Complete CMake migration
- [ ] Implement binary caching
- [ ] Optimize platform-specific builds

## Expected Results

### Performance Targets
- Quick Test: < 2 minutes (40% improvement)
- Full Build: < 5 minutes (50% improvement)
- Total Pipeline: < 10 minutes (60% improvement)

### Quality Targets
- No reduction in test coverage
- Maintain all platform support
- Improve developer feedback time

### Cost Targets
- Reduce GitHub Actions minutes by 50%
- Lower storage usage through better caching
- Minimize redundant computations

## Monitoring Success

### Key Metrics
1. **Build Time**: Average time per commit
2. **Cache Hit Rate**: Percentage of cached vs fresh builds
3. **Failure Rate**: Percentage of failed builds
4. **Resource Usage**: CPU/Memory/Storage per build

### Dashboards
Create dashboards showing:
- Build time trends
- Success/failure rates
- Platform-specific metrics
- Cost analysis

## Rollback Plan

Each optimization should be:
- Feature-flagged when possible
- Tested in separate branch first
- Documented with before/after metrics
- Reversible without data loss

## Conclusion

This optimization plan provides a structured approach to improving CI/CD performance while maintaining quality. The phased approach allows for incremental improvements with measurable results at each stage.