# LPZRobots API Compatibility Reference

## Critical API Mappings for Testing

This document captures all the API compatibility issues discovered and resolved during the doctest migration, serving as a reference for future test development.

## 📐 Matrix Operations

### Basic Operations

| Operation | Old/Wrong API | Correct API | Notes |
|-----------|---------------|-------------|-------|
| Transpose | `m.T()` | `m^T` | Operator overload |
| Identity | `Matrix::eye(n)` | `Matrix m(n,n); m.toId()` | No static method |
| Set all values | `m.set(value)` | `Matrix(rows, cols, value)` | Use constructor |
| Norm | `m.norm()` | `sqrt(m.norm_sqr())` | Only squared norm available |
| Zero matrix | `Matrix::zeros(r,c)` | `Matrix m(r,c); m.toZero()` | No static method |

### Matrix Construction

```cpp
// WRONG
Matrix m(size);          // No single-dimension constructor
Matrix m = Matrix(10);   // Won't compile

// CORRECT
Matrix m(rows, cols);    // Always specify both dimensions
Matrix m(10, 1);         // Column vector
Matrix m(1, 10);         // Row vector
```

### Advanced Operations

```cpp
// Pseudo-inverse
Matrix pinv = m.pseudoInverse();  // Correct

// Element-wise operations
Matrix c = a & b;    // Element-wise multiplication
Matrix c = a | b;    // Element-wise division

// Matrix multiplication variants
Matrix c = a * b;           // Standard multiplication
Matrix c = a.multMT();      // a * b^T
Matrix c = a.multTM();      // a^T * b

// Accessing elements
double val = m.val(i, j);        // Get value
m.val(i, j) = 1.0;              // Set value
const double* data = m.getData(); // Get raw data pointer
```

## 🎮 Controller APIs

### Controller Factory

```cpp
// WRONG
auto controller = ControllerFactory::createController("Sox");

// CORRECT
auto controller = lpzrobots::ControllerFactory::createController("Sox");
```

### Common Parameters

| Controller | Parameter | Type | Typical Range | Description |
|------------|-----------|------|---------------|-------------|
| Sox | epsC | double | 0.01-0.5 | Controller learning rate |
| Sox | epsA | double | 0.0-0.2 | Model learning rate |
| DEP | epsC | double | 0.01-1.0 | Learning rate |
| DEP | epsh | double | 0.0-0.1 | Bias learning rate |
| DEP | s4avg | int | 1-10 | Averaging window |
| Sos | epsC | double | 0.01-0.5 | Controller learning rate |

### Controller Initialization

```cpp
// Standard initialization sequence
controller->init(sensornum, motornum, &randGen);

// Getting values
Matrix x = controller->getLastSensorValues();
Matrix y = controller->getLastMotorValues();

// Parameter management
controller->setParam("epsC", 0.1);
double eps = controller->getParam("epsC");
std::list<Configurable::paramkey> params = controller->getAllParamNames();
```

## 🔌 Wiring APIs

### NoiseGenerator Usage

```cpp
// WRONG
auto wiring = std::make_unique<One2OneWiring>(NoNoise, 3);

// CORRECT
auto wiring = std::make_unique<One2OneWiring>(new NoNoise(), 3);
```

### Wiring Methods

```cpp
// Wire sensors - note the noise strength parameter!
bool success = wiring->wireSensors(
    rsensors, rsensornumber,      // Robot sensors
    csensors, csensornumber,      // Controller sensors
    noiseStrength                 // REQUIRED: typically 1.0
);

// Wire motors
bool success = wiring->wireMotors(
    cmotors, cmotornumber,        // Controller motors
    rmotors, rmotornumber         // Robot motors
);
```

### Derivative Wiring Configuration

```cpp
// WRONG
auto wiring = std::make_unique<DerivativeWiring>(new NoNoise(), true);

// CORRECT
DerivativeWiringConf conf;
conf.useId = true;           // Include position
conf.useFirstD = true;       // Include velocity
conf.eps = 0.1;             // Time step for derivative
conf.derivativeScale = 1.0;  // Scaling factor
auto wiring = std::make_unique<DerivativeWiring>(conf, new NoNoise());
```

## 🤖 Robot APIs

### AbstractRobot Implementation

```cpp
class MockRobot : public AbstractRobot {
public:
    // WRONG - const qualifiers don't match
    virtual int getSensorNumber() const override;
    
    // CORRECT - must match base class exactly
    virtual int getSensorNumber() override;
    virtual int getMotorNumber() override;
    virtual int getSensors(sensor* sensors, int sensornumber) override;
    virtual void setMotors(const motor* motors, int motornumber) override;
};
```

### Required Virtual Methods

```cpp
// Position and orientation
virtual Position getPosition() const override;
virtual Position getSpeed() const override;
virtual Position getAngularSpeed() const override;
virtual matrix::Matrix getOrientation() const override;
```

## ⚙️ Physics Simulation APIs

### Joint Construction

```cpp
// HingeJoint
HingeJoint hinge(
    primitive1, primitive2,  // Connected bodies
    anchor,                  // Anchor point (Pos)
    axis                    // Rotation axis (Axis)
);

// SliderJoint - needs anchor!
SliderJoint slider(
    primitive1, primitive2,
    anchor,                 // REQUIRED anchor point
    axis                   // Sliding axis
);

// UniversalJoint
UniversalJoint universal(
    primitive1, primitive2,
    anchor,                // Anchor point
    axis1,                // First axis
    axis2                 // Second axis
);
```

### Joint Operations

```cpp
// WRONG
hinge.setPosition1(angle);      // Method doesn't exist

// CORRECT
hinge.setParam(dParamVel, velocity);
hinge.setParam(dParamFMax, maxForce);
double pos = hinge.getPosition1();
hinge.addForce1(force);
```

### Primitive Dimensions

```cpp
// WRONG
Pos dims = box.getDim();

// CORRECT
const OSGPrimitive* osgPrim = box.getOSGPrimitive();
if (const OSGBoxTex* osgBox = dynamic_cast<const OSGBoxTex*>(osgPrim)) {
    osg::Vec3 dims = osgBox->getDim();
}
```

### Box Fixation

```cpp
// WRONG
box.fixate();  // Method doesn't exist

// CORRECT - Set mass to 0
dMass mass;
dMassSetBoxTotal(&mass, 0, width, height, depth);
dBodySetMass(box.getBody(), &mass);
```

## 📝 Doctest Specific

### Assertion Patterns

```cpp
// Simple comparisons work best
CHECK(value > 0);
CHECK(value == expected);

// Complex expressions need simplification
// WRONG
CHECK(a > 0 && b < 10 && c == 5);

// CORRECT
bool condition = (a > 0) && (b < 10) && (c == 5);
CHECK(condition);
```

### Floating Point Comparisons

```cpp
// Use doctest::Approx
CHECK(value == doctest::Approx(expected));
CHECK(value == doctest::Approx(expected).epsilon(0.01));

// Custom comparison function
bool approxEqual(double a, double b, double eps = 1e-10) {
    return std::abs(a - b) < eps;
}
```

### Skip Tests

```cpp
// WRONG
SKIP("Message");

// CORRECT
MESSAGE("Test skipped: reason");
return;
```

## 🚨 Common Pitfalls

1. **Matrix dimensions**: Always use (rows, cols), never single dimension
2. **Noise generators**: Always use `new NoNoise()`, not `NoNoise`
3. **Wire sensors**: Don't forget the noise strength parameter
4. **Controller namespace**: Use `lpzrobots::` prefix
5. **Joint methods**: Check what actually exists in the API
6. **Override signatures**: Must match base class exactly

## 🔍 Debugging Tips

1. **Segmentation faults**: Usually matrix dimension mismatches
2. **Pure virtual errors**: Missing override implementations
3. **Undefined symbols**: Missing library dependencies
4. **Type mismatches**: Check const qualifiers carefully
5. **Workspace crashes**: Simplify matrix operations

This reference captures the hard-won knowledge from fixing hundreds of API compatibility issues during the migration. Use it to write correct tests from the start!