#include <selforg/abstractcontroller.h>
#include <selforg/matrix.h>
#include <iostream>
#include <memory>

class TestController : public AbstractController {
public:
    TestController() : AbstractController("TestController", "$Id$") {}
    
    void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override {
        number_sensors = sensornumber;
        number_motors = motornumber;
    }
    
    void step(const sensor* sensors, int number_sensors,
              motor* motors, int number_motors) override {
        // Simple test: copy sensor values to motors
        for(int i = 0; i < number_motors && i < number_sensors; i++) {
            motors[i] = sensors[i];
        }
    }
    
    void stepNoLearning(const sensor* sensors, int number_sensors,
                        motor* motors, int number_motors) override {
        step(sensors, number_sensors, motors, number_motors);
    }
    
    int getSensorNumber() const override { return number_sensors; }
    int getMotorNumber() const override { return number_motors; }
    
    bool store(FILE* f) const { return true; }
    bool restore(FILE* f) { return true; }

private:
    int number_sensors = 0;
    int number_motors = 0;
};

int main() {
    std::cout << "Testing LPZRobots with sanitizers..." << std::endl;
    
    // Test 1: Basic controller functionality
    auto controller = std::make_unique<TestController>();
    controller->init(2, 2);
    
    double sensors[2] = {0.5, -0.5};
    double motors[2] = {0.0, 0.0};
    
    controller->step(sensors, 2, motors, 2);
    
    std::cout << "Sensor values: " << sensors[0] << ", " << sensors[1] << std::endl;
    std::cout << "Motor values: " << motors[0] << ", " << motors[1] << std::endl;
    
    // Test 2: Matrix operations
    matrix::Matrix m(2, 2);
    m.val(0, 0) = 1.0;
    m.val(0, 1) = 2.0;
    m.val(1, 0) = 3.0;
    m.val(1, 1) = 4.0;
    
    matrix::Matrix m2 = m * m;
    std::cout << "Matrix multiplication result:" << std::endl;
    std::cout << m2 << std::endl;
    
    std::cout << "All tests completed successfully!" << std::endl;
    return 0;
}