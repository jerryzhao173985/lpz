# Test Coverage Improvement Plan for LPZRobots

## Current State Analysis

### What We Have
1. **Build Testing**: ✅ Excellent (multiple platforms, configurations)
2. **Static Analysis**: ✅ Comprehensive (clang-tidy, cppcheck)
3. **Memory Safety**: ✅ Strong (ASAN, UBSAN)
4. **Performance**: ⚠️ Basic (only matrix operations)
5. **Unit Tests**: ❌ Very limited
6. **Integration Tests**: ⚠️ Basic simulation builds only
7. **Functional Tests**: ❌ None

### Coverage Gaps
- No tests for controller algorithms
- No physics simulation validation
- No GUI testing
- Limited matrix operation tests
- No regression tests for scientific accuracy

## Proposed Test Suite Structure

### 1. Unit Tests (Priority: HIGH)

#### Matrix Operations
```cpp
// selforg/tests/test_matrix_operations.cpp
TEST(MatrixTest, Multiplication) {
    Matrix A(2, 2); A.val(0,0) = 1; A.val(0,1) = 2;
                    A.val(1,0) = 3; A.val(1,1) = 4;
    Matrix B = A * A;
    EXPECT_EQ(B.val(0,0), 7);
    EXPECT_EQ(B.val(1,1), 22);
}

TEST(MatrixTest, Inversion) {
    Matrix A(2, 2); A.toId();
    Matrix B = A.pseudoInverse();
    EXPECT_TRUE((A * B).isIdentity());
}
```

#### Controller Tests
```cpp
// selforg/tests/test_controllers.cpp
TEST(SoxTest, BasicStep) {
    Sox controller;
    controller.init(2, 2);
    
    sensor sensors[2] = {0.5, -0.5};
    motor motors[2];
    controller.step(sensors, 2, motors, 2);
    
    EXPECT_NE(motors[0], 0.0);
    EXPECT_NE(motors[1], 0.0);
}

TEST(SoxTest, LearningRate) {
    Sox controller;
    controller.setParam("epsA", 0.1);
    EXPECT_EQ(controller.getParam("epsA"), 0.1);
}
```

### 2. Integration Tests (Priority: HIGH)

#### Simulation Tests
```cpp
// ode_robots/tests/test_simulations.cpp
TEST(SimulationTest, SpherebotMovement) {
    GlobalData global;
    Spherebot3Masses* robot = new Spherebot3Masses(/*...*/);
    Sox* controller = new Sox();
    
    OdeAgent* agent = new OdeAgent(global);
    agent->init(controller, robot, new One2OneWiring());
    
    // Run 100 steps
    for(int i = 0; i < 100; i++) {
        agent->step(0.01);
    }
    
    // Robot should have moved
    Position pos = robot->getPosition();
    EXPECT_NE(pos.x, 0.0);
}
```

### 3. Performance Benchmarks (Priority: MEDIUM)

#### Controller Performance
```cpp
// selforg/benchmarks/bench_controllers.cpp
static void BM_SoxStep(benchmark::State& state) {
    Sox controller;
    controller.init(state.range(0), state.range(0));
    
    sensor* sensors = new sensor[state.range(0)];
    motor* motors = new motor[state.range(0)];
    
    for (auto _ : state) {
        controller.step(sensors, state.range(0), 
                       motors, state.range(0));
    }
    
    delete[] sensors;
    delete[] motors;
}
BENCHMARK(BM_SoxStep)->Range(2, 128);
```

### 4. Physics Validation Tests (Priority: MEDIUM)

```cpp
// ode_robots/tests/test_physics.cpp
TEST(PhysicsTest, GravityFall) {
    // Create sphere at height 10
    Sphere sphere(0.1);
    sphere.setPosition(Pos(0, 0, 10));
    
    // Simulate 1 second
    for(int i = 0; i < 100; i++) {
        dWorldStep(world, 0.01);
    }
    
    // Should fall approximately 4.9 meters
    EXPECT_NEAR(sphere.getPosition().z, 5.1, 0.1);
}
```

### 5. Regression Tests (Priority: LOW)

```cpp
// selforg/tests/test_regression.cpp
TEST(RegressionTest, SoxBehavior) {
    // Load recorded sensor/motor data
    auto data = loadTestData("sox_regression_data.txt");
    
    Sox controller;
    controller.init(2, 2);
    
    for(auto& frame : data) {
        motor motors[2];
        controller.step(frame.sensors, 2, motors, 2);
        
        // Compare with recorded behavior
        EXPECT_NEAR(motors[0], frame.expected_motors[0], 0.01);
        EXPECT_NEAR(motors[1], frame.expected_motors[1], 0.01);
    }
}
```

## Implementation Plan

### Phase 1: Foundation (Week 1)
1. Set up Google Test framework
2. Create test directories and CMake integration
3. Write basic matrix operation tests
4. Add tests to CI pipeline

### Phase 2: Core Tests (Week 2)
1. Controller unit tests (Sox, Sos, DEP)
2. Wiring tests
3. Basic physics tests
4. Integration with coverage reporting

### Phase 3: Advanced Tests (Week 3)
1. Performance benchmarks
2. Simulation integration tests
3. Memory leak tests
4. Thread safety tests

### Phase 4: Scientific Validation (Week 4)
1. Regression test suite
2. Numerical accuracy tests
3. Long-running stability tests
4. Parameter sweep tests

## CI Integration

### Update CMakeLists.txt
```cmake
# Enable testing
enable_testing()
include(GoogleTest)

# Add test subdirectories
add_subdirectory(selforg/tests)
add_subdirectory(ode_robots/tests)

# Coverage target
if(LPZROBOTS_ENABLE_COVERAGE)
    setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE ctest
        EXCLUDE "/usr/*" "*/tests/*"
    )
endif()
```

### Update CI Workflow
```yaml
- name: Run comprehensive tests
  run: |
    cd build
    ctest --output-on-failure --parallel $(nproc)
    
- name: Generate coverage report
  if: matrix.coverage == true
  run: |
    cd build
    make coverage
    
- name: Upload coverage to Codecov
  uses: codecov/codecov-action@v3
  with:
    files: ./build/coverage.info
```

## Success Metrics

### Coverage Goals
- **Line Coverage**: 70% (from ~40%)
- **Function Coverage**: 80%
- **Branch Coverage**: 60%

### Test Execution Time
- Unit tests: < 30 seconds
- Integration tests: < 2 minutes
- Full test suite: < 5 minutes

### Quality Metrics
- Zero test flakiness
- All tests pass on all platforms
- Clear test documentation
- Fast feedback on failures

## Benefits

1. **Confidence**: Know that changes don't break existing functionality
2. **Documentation**: Tests serve as usage examples
3. **Refactoring**: Safe to improve code with test coverage
4. **Performance**: Track performance regressions
5. **Science**: Validate scientific accuracy of algorithms

## Next Steps

1. Get buy-in from maintainers
2. Set up testing framework
3. Start with high-value tests
4. Gradually increase coverage
5. Make tests required for PRs

This plan provides a roadmap to significantly improve the test coverage and quality assurance for LPZRobots, making it more reliable and maintainable for research use.