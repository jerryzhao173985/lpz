# Legacy Make Deprecation Assessment

## Date: 2025-01-11

## Executive Summary

After successfully fixing the Legacy Make CI build, we should consider whether to deprecate the Legacy Make system in favor of the modern CMake build system.

## Current State

### CI Build Status
- **CMake builds**: 5/5 passing ✅
- **Legacy Make build**: 1/1 passing ✅ (after extensive fixes)

### Maintenance Effort

#### Legacy Make System
- Complex M4 templating system
- Manual dependency management
- Platform-specific hacks and workarounds
- Hardcoded paths causing CI issues
- Significant time spent debugging build issues
- Mode switching complexity (DEVEL vs USER)

#### CMake System
- Modern dependency management
- Cross-platform support out of the box
- Standard tooling and IDE integration
- Easier to debug and maintain
- Better documentation and community support

## Pros and Cons

### Keeping Legacy Make

**Pros:**
- Historical compatibility for long-time users
- Some researchers may have scripts depending on it
- Now working in CI after fixes

**Cons:**
- High maintenance burden
- Complex and error-prone
- Limited to Unix-like systems
- No modern IDE integration
- Difficult for new contributors

### Deprecating Legacy Make

**Pros:**
- Reduced maintenance burden
- Single build system to support
- Better developer experience
- Modern tooling support
- Easier onboarding for new contributors

**Cons:**
- May break existing workflows
- Requires migration documentation
- Some users need to learn CMake

## Recommendation

**Deprecate Legacy Make with a transition period:**

### Phase 1: Soft Deprecation (3-6 months)
1. Add deprecation notice to Legacy Make
2. Update documentation to prioritize CMake
3. Provide migration guide
4. Keep Legacy Make working but unsupported

### Phase 2: Hard Deprecation (6-12 months)
1. Move Legacy Make to `legacy/` directory
2. Remove from CI
3. Archive documentation
4. Focus all efforts on CMake

### Migration Support
1. Create comprehensive migration guide
2. Provide CMake equivalents for common Make commands
3. Document differences in installation paths
4. Offer support during transition period

## Implementation Steps

1. **Immediate Actions:**
   - Add deprecation notice to Makefile
   - Update README to recommend CMake
   - Create MIGRATION.md guide

2. **Short Term (1-3 months):**
   - Ensure CMake has feature parity
   - Test all example simulations with CMake
   - Update all documentation

3. **Medium Term (3-6 months):**
   - Remove Legacy Make from main documentation
   - Move to maintenance-only mode
   - Stop adding new features

4. **Long Term (6-12 months):**
   - Archive Legacy Make system
   - Remove from main branch
   - Full commitment to CMake

## Conclusion

The Legacy Make system has served its purpose but has become a maintenance burden. With CMake providing a superior alternative that's already working well, deprecation is the logical next step. A phased approach will minimize disruption while modernizing the build system.

## Decision Required

This assessment recommends deprecating Legacy Make. The project maintainers should review and make a final decision based on:
- User feedback
- Resource availability
- Project priorities
- Community needs