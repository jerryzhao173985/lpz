# CI/CD Optimization Plan for LPZRobots

## Overview

This document outlines a comprehensive optimization strategy to improve CI/CD performance, reduce build times, and enhance developer experience.

## Current Performance Metrics

### Baseline Measurements
- **Quick Test Job**: ~5 minutes
- **Full Build (Debug)**: ~15-20 minutes
- **Full Build (Release)**: ~12-15 minutes
- **Legacy Make Build**: ~25-30 minutes
- **Total Pipeline Time**: ~45-60 minutes

### Resource Usage
- **CPU**: Not fully utilized (serial builds in some components)
- **Memory**: Adequate (no OOM issues)
- **Storage**: ~500MB artifacts per build
- **Network**: Package downloads on every run

## Optimization Goals

1. **Reduce Total Pipeline Time**: Target < 20 minutes
2. **Improve Feedback Speed**: First results in < 3 minutes
3. **Reduce Resource Usage**: 50% less network/storage
4. **Enhance Reliability**: < 5% flaky test rate

## Optimization Strategies

### Phase 1: Quick Wins (1-2 weeks)

#### 1.1 Enhanced Caching

**Current State**: Basic ccache, minimal artifact caching

**Improvements**:
```yaml
- name: Advanced Cache Setup
  uses: actions/cache@v3
  with:
    path: |
      ~/.cache/ccache
      ~/lpzrobots
      build/**/*.a
      build/**/*.o
    key: ${{ runner.os }}-${{ matrix.build_type }}-${{ hashFiles('**/*.cpp', '**/*.h') }}
    restore-keys: |
      ${{ runner.os }}-${{ matrix.build_type }}-
      ${{ runner.os }}-
```

**Expected Impact**: 30-40% build time reduction

#### 1.2 Parallel Job Optimization

**Current Issues**:
- Serial component builds
- Unnecessary dependencies

**Solution**:
```yaml
strategy:
  matrix:
    component: [selforg, ode_robots, ga_tools, gui_tools]
  
- name: Build Component
  run: |
    cmake --build build --target ${{ matrix.component }} -j$(nproc)
```

**Expected Impact**: 25% faster builds

#### 1.3 Dependency Caching

**Implementation**:
```yaml
- name: Cache Dependencies
  uses: actions/cache@v3
  with:
    path: |
      /opt/homebrew/Cellar
      /usr/local/Cellar
      ~/.conan
    key: deps-${{ runner.os }}-${{ hashFiles('conanfile.txt') }}
```

**Expected Impact**: 5-10 minute reduction in setup time

### Phase 2: Architecture Improvements (2-4 weeks)

#### 2.1 Build Matrix Optimization

**Current**: All combinations run always

**Optimized Strategy**:
```yaml
jobs:
  quick-check:
    # Fast sanity check
    runs-on: ubuntu-latest
    steps:
      - name: Quick Build
        run: cmake --build build --target selforg
  
  full-build:
    needs: quick-check
    if: success()
    strategy:
      matrix:
        include:
          # Minimal matrix for PRs
          - os: ubuntu-latest
            build_type: Debug
          # Full matrix for main branch
          - os: [ubuntu-latest, macos-latest]
            build_type: [Debug, Release]
            if: github.ref == 'refs/heads/main'
```

#### 2.2 Incremental Builds

**Implementation**:
- Use `ccache` statistics to optimize
- Implement build artifact reuse
- Smart change detection

```bash
# Detect changed components
CHANGED_COMPONENTS=$(git diff --name-only HEAD~1 | grep -E "^(selforg|ode_robots)" | cut -d'/' -f1 | sort -u)
for component in $CHANGED_COMPONENTS; do
  cmake --build build --target $component
done
```

#### 2.3 Test Parallelization

**Current**: Serial test execution

**Optimization**:
```cmake
# In CMakeLists.txt
set(CTEST_PARALLEL_LEVEL ${CMAKE_BUILD_PARALLEL_LEVEL})
```

```yaml
# In CI
- name: Run Tests in Parallel
  run: ctest --parallel $(nproc) --output-on-failure
```

### Phase 3: Advanced Optimizations (1-2 months)

#### 3.1 Distributed Builds

**Options**:
1. **GitHub Actions Matrix**
   - Split build across multiple runners
   - Merge artifacts at end

2. **distcc Integration**
   ```yaml
   - name: Setup distcc
     run: |
       sudo apt-get install distcc
       export DISTCC_HOSTS="localhost/$(nproc)"
       export CC="distcc gcc"
       export CXX="distcc g++"
   ```

3. **Bazel Migration** (long-term)
   - Hermetic builds
   - Perfect caching
   - Remote execution

#### 3.2 Docker Layer Caching

**Implementation**:
```dockerfile
# Multi-stage Dockerfile
FROM ubuntu:24.04 AS dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake libgsl-dev ...

FROM dependencies AS builder
COPY . /src
WORKDIR /src
RUN cmake -B build && cmake --build build

FROM dependencies AS runtime
COPY --from=builder /src/build/install /opt/lpzrobots
```

```yaml
# CI usage
- name: Build with Docker
  uses: docker/build-push-action@v4
  with:
    cache-from: type=gha
    cache-to: type=gha,mode=max
```

#### 3.3 Smart Test Selection

**Concept**: Only run tests affected by changes

**Implementation**:
```python
# test_selector.py
import subprocess
import json

def get_affected_tests(changed_files):
    # Map files to test targets
    test_map = {
        'selforg/': ['selforg_tests'],
        'ode_robots/': ['ode_robots_tests'],
    }
    
    affected_tests = set()
    for file in changed_files:
        for pattern, tests in test_map.items():
            if file.startswith(pattern):
                affected_tests.update(tests)
    
    return list(affected_tests)
```

### Phase 4: Infrastructure Optimization (2-3 months)

#### 4.1 Self-Hosted Runners

**Benefits**:
- Persistent cache
- More powerful hardware
- Custom environment

**Setup**:
```yaml
runs-on: [self-hosted, linux, x64]
```

#### 4.2 Build Farm

**Architecture**:
```
┌─────────────┐     ┌──────────────┐
│   GitHub    │────▶│ Coordinator  │
│   Actions   │     └──────┬───────┘
└─────────────┘            │
                           ▼
        ┌──────────────────────────────┐
        │      Build Workers          │
        ├────────┬────────┬──────────┤
        │ Linux  │ macOS  │ Windows  │
        │ x64    │ ARM64  │   x64   │
        └────────┴────────┴──────────┘
```

#### 4.3 Artifact CDN

**Implementation**:
- Use S3/CloudFront for artifact storage
- Implement smart retention policies
- Geographic distribution

## Implementation Timeline

### Week 1-2: Quick Wins
- [ ] Implement enhanced caching
- [ ] Enable parallel builds everywhere
- [ ] Add dependency caching

### Week 3-4: Testing
- [ ] Measure performance improvements
- [ ] Fix any regressions
- [ ] Document new procedures

### Month 2: Architecture
- [ ] Implement build matrix optimization
- [ ] Add incremental build support
- [ ] Enable test parallelization

### Month 3: Advanced Features
- [ ] Docker layer caching
- [ ] Smart test selection
- [ ] Performance dashboard

### Month 4+: Infrastructure
- [ ] Evaluate self-hosted runners
- [ ] Design build farm architecture
- [ ] Implement artifact CDN

## Expected Outcomes

### Performance Improvements

| Metric | Current | Target | Improvement |
|--------|---------|--------|-------------|
| Quick Test | 5 min | 2 min | 60% |
| Full Build | 20 min | 8 min | 60% |
| Total Pipeline | 45 min | 15 min | 67% |
| First Feedback | 5 min | 1 min | 80% |

### Cost Reduction

- **Compute Time**: 60% reduction
- **Storage**: 50% reduction (smart retention)
- **Network**: 70% reduction (caching)

### Developer Experience

- **Faster Feedback**: < 2 minutes for basic validation
- **Parallel Development**: Multiple PRs without queue
- **Better Debugging**: Enhanced logs and artifacts

## Monitoring and Metrics

### Key Performance Indicators

1. **Build Time Metrics**
   - P50, P90, P99 build times
   - Component-specific timings
   - Cache hit rates

2. **Reliability Metrics**
   - Success rate
   - Flaky test percentage
   - Infrastructure failures

3. **Resource Metrics**
   - CPU utilization
   - Memory usage
   - Network bandwidth

### Implementation

```yaml
- name: Collect Metrics
  if: always()
  run: |
    echo "BUILD_TIME=$((END_TIME - START_TIME))" >> metrics.txt
    echo "CACHE_HIT_RATE=$(ccache -s | grep 'cache hit rate')" >> metrics.txt
    
- name: Upload Metrics
  uses: actions/upload-artifact@v4
  with:
    name: ci-metrics
    path: metrics.txt
```

## Risk Mitigation

### Potential Risks

1. **Complexity Increase**
   - Mitigation: Gradual rollout, good documentation

2. **Flaky Tests**
   - Mitigation: Test isolation, retry mechanisms

3. **Cache Corruption**
   - Mitigation: Cache versioning, validation

4. **Breaking Changes**
   - Mitigation: Feature flags, rollback procedures

## Conclusion

This optimization plan provides a roadmap to significantly improve CI/CD performance. The phased approach ensures minimal disruption while delivering immediate value. Regular monitoring and adjustment will ensure continued optimization as the project evolves.