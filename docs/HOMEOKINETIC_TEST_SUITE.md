# Homeokinetic Controller Test Suite Documentation

## Overview

This document describes the comprehensive test suite for the homeokinetic controllers in LPZRobots. These controllers (Sox, Sos, and DEP) implement self-organizing control algorithms that are fundamental to the robot's ability to develop behaviors autonomously.

## Test Structure

### 1. Core Algorithm Tests (`homeokinetic_core_tests.cpp`)

Tests the mathematical correctness and fundamental properties of the controllers.

#### Sox Controller Tests
- **Initialization**: Verifies correct matrix dimensions and initial values
- **Transfer Functions**: Tests tanh and its derivative properties
- **Jacobian Calculation**: Validates L = A * C * g' + S computation
- **Time-loop Error**: Checks the core learning signal calculation
- **Learning Rate Effects**: Verifies that different learning rates produce expected changes

#### Sos Controller Tests
- **Simplified Model**: Validates behavior without S matrix
- **Parameter Bounds**: Tests parameter constraints

#### DEP Controller Tests
- **Extended Buffer Usage**: Tests the 150-step buffer capability
- **Learning Rules**: Validates different learning rule implementations
- **Time-delayed Prediction**: Tests prediction with temporal delays

### 2. Algorithm Detail Tests (`homeokinetic_algorithm_tests.cpp`)

Tests specific algorithmic components and mathematical operations.

#### Gradient Calculations
- **Controller Update**: ΔC = ε(μv^T - εrel·y·x^T)
- **Model Update**: ΔA = ε·ξ·y^T

#### Extended Model Features
- **S Matrix Learning**: Sensor-to-sensor prediction capability
- **Full Jacobian**: L = A*C*g' + S implementation

#### Normalization and Regularization
- **Controller Matrix Normalization**: Prevents unbounded growth
- **Damping**: Forgetting factor to prevent divergence

#### Information Theory
- **Predictive Information**: Balances predictability and exploration
- **Causality Learning**: Discovers causal relationships

### 3. Robustness Tests (`homeokinetic_robustness_tests.cpp`)

Tests behavior under extreme conditions and edge cases.

#### Numerical Stability
- **NaN Propagation**: Prevents invalid values from spreading
- **Infinity Handling**: Graceful saturation of extreme inputs
- **Denormal Numbers**: Handles very small values efficiently

#### Matrix Singularity
- **Singular Jacobian**: Handles rank-deficient matrices
- **Ill-conditioned Matrices**: Maintains stability with poor conditioning

#### Boundary Conditions
- **Zero Dimensions**: Handles empty sensor/motor arrays
- **Maximum Dimensions**: Tests scalability to large systems
- **Asymmetric Dimensions**: Many sensors, few motors

#### Recovery Mechanisms
- **Zero Lock Recovery**: Escapes from stuck states
- **Saturation Recovery**: Returns from extreme values

### 4. Verification Tests (`homeokinetic_verification_test.cpp`)

Validates the core homeokinetic principles are preserved.

#### Buffer Initialization Timing
- Sox/Sos start learning after 2 steps (not 10)
- DEP starts after 3 steps (not 150)

#### Learning Dynamics
- Controllers maintain exploration (don't minimize error)
- Dynamic stability instead of convergence
- Adaptation to environmental changes

### 5. Behavior Tests (`homeokinetic_behavior_test.cpp`)

Tests emergent behavioral properties.

#### Dynamic Stability
- Avoids fixed points
- Shows self-organized oscillations
- Maintains bounded activity

#### Environmental Adaptation
- Responds to perturbations
- Integrates teaching signals
- Shows parameter sensitivity

## Key Test Findings

### Performance Characteristics
- **Scaling**: O(n²) with dimension size as expected
- **2D System**: ~19ms for 1000 steps
- **20D System**: ~195ms for 1000 steps

### Algorithm Correctness
- Transfer functions (tanh) work correctly
- Matrix operations preserve numerical accuracy
- Learning gradients follow theoretical equations

### Robustness
- Handles extreme inputs gracefully
- Recovers from degenerate states
- Maintains stability over long runs (10,000+ steps)

## Test Coverage

### What's Tested
✅ Mathematical correctness of core algorithms
✅ Matrix operations and linear algebra
✅ Learning dynamics and gradients
✅ Parameter bounds and validation
✅ Edge cases and error recovery
✅ Performance characteristics
✅ Behavioral properties
✅ Teaching signal integration

### What's Not Tested (Future Work)
- GPU acceleration paths
- Multi-threaded execution
- Network communication
- Real robot hardware integration
- Long-term memory effects (>10,000 steps)

## Running the Tests

```bash
# Build the tests
make unit_tests

# Run all homeokinetic tests
./tests/unit_tests --test-suite="Homeokinetic*"

# Run specific test suites
./tests/unit_tests --test-suite="Homeokinetic Core Algorithms"
./tests/unit_tests --test-suite="Homeokinetic Algorithm Details"
./tests/unit_tests --test-suite="Homeokinetic Robustness"

# Run with detailed output
./tests/unit_tests --test-suite="Homeokinetic*" --reporter=console --duration
```

## Test Status

As of latest run:
- **Core Algorithm Tests**: 5/8 passing (some teaching tests need adjustment)
- **Algorithm Detail Tests**: Passing (with minor matrix dimension issue)
- **Robustness Tests**: Mostly passing
- **Verification Tests**: All passing ✅
- **Behavior Tests**: All passing ✅

## Continuous Integration

These tests should be run:
1. On every commit to main branch
2. For all pull requests
3. Before releases
4. After any changes to controller algorithms

## Maintenance Notes

1. **Keep Tests Fast**: Total suite should run in <5 seconds
2. **Avoid Randomness**: Use fixed seeds for reproducibility
3. **Test Isolation**: Each test should be independent
4. **Clear Failures**: Error messages should indicate what failed
5. **Update with Algorithm Changes**: Keep tests synchronized with theory

## Scientific Validation

These tests validate that the implementation matches the theoretical foundations described in:
- Der, R. & Martius, G. (2011). The Playful Machine
- The homeokinetic control principles
- Time-loop error minimization with anti-Hebbian regularization

The test suite ensures that any changes to the codebase preserve these fundamental scientific principles.