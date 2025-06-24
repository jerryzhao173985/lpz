// C++17 Modernization Example for Controller Classes
// Shows migration from old C++ style to modern C++17

// ===== BEFORE (Old Style) =====
/*
// Old abstractcontroller.h
class AbstractController : public Configurable, public Inspectable, public Storeable {
public:
    typedef double sensor;
    typedef double motor;
    
    AbstractController(const std::string& name, const std::string& revision)
        : Configurable(name, revision), Inspectable(name) {}
        
    virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) = 0;
    virtual int getSensorNumber() const = 0;
    virtual int getMotorNumber() const = 0;
    
    virtual void step(const sensor* sensors, int sensornumber,
                     motor* motors, int motornumber) = 0;
                     
    virtual void sensorInfos(std::list<SensorMotorInfo> sensorInfos);
    
protected:
    std::map<std::string, int> sensorIndexMap;
    std::map<std::string, SensorMotorInfo> sensorInfoMap;
};

// Old implementation
class BasicController : public AbstractController {
    double* weights;
    int sensornumber;
    int motornumber;
    
public:
    BasicController() 
        : AbstractController("BasicController", "$Id$"), 
          weights(0), sensornumber(0), motornumber(0) {}
          
    ~BasicController() {
        if(weights) delete[] weights;
    }
    
    virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) {
        this->sensornumber = sensornumber;
        this->motornumber = motornumber;
        
        if(weights) delete[] weights;
        weights = new double[sensornumber * motornumber];
        
        for(int i = 0; i < sensornumber * motornumber; ++i) {
            weights[i] = randGen ? randGen->rand() : 0.1;
        }
    }
};
*/

// ===== AFTER (Modern C++17) =====
#ifndef MODERN_CONTROLLER_H
#define MODERN_CONTROLLER_H

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <memory>
#include <algorithm>
#include <numeric>
#include <optional>
#include <variant>
#include <span>
#include <unordered_map>

// Modern type aliases using 'using' instead of typedef
using sensor_t = double;
using motor_t = double;

// Use concepts when C++20 is available
#ifdef __cpp_concepts
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;
#endif

// Modern abstract controller with C++17 features
class AbstractController : public Configurable, public Inspectable, public Storeable {
public:
    // Delete copy operations, allow move
    AbstractController(const AbstractController&) = delete;
    AbstractController& operator=(const AbstractController&) = delete;
    AbstractController(AbstractController&&) = default;
    AbstractController& operator=(AbstractController&&) = default;
    
    // Constructor with string_view for efficiency
    AbstractController(std::string_view name, std::string_view revision)
        : Configurable(std::string(name), std::string(revision))
        , Inspectable(std::string(name)) {}
    
    // Virtual destructor with override
    virtual ~AbstractController() = default;
    
    // Modern interface with optional and span
    virtual void init(int sensornumber, int motornumber, 
                     std::optional<std::reference_wrapper<RandGen>> randGen = std::nullopt) = 0;
    
    // Use [[nodiscard]] for getters
    [[nodiscard]] virtual int getSensorNumber() const noexcept = 0;
    [[nodiscard]] virtual int getMotorNumber() const noexcept = 0;
    
    // Use span for array parameters (C++20 feature, fallback for C++17)
#ifdef __cpp_lib_span
    virtual void step(std::span<const sensor_t> sensors,
                     std::span<motor_t> motors) = 0;
#else
    // C++17 fallback using pointer and size
    virtual void step(const sensor_t* sensors, int sensornumber,
                     motor_t* motors, int motornumber) = 0;
#endif
    
    // Pass by value for sink parameters (small objects)
    virtual void sensorInfos(std::vector<SensorMotorInfo> sensorInfos);
    
protected:
    // Use unordered_map for O(1) lookup
    std::unordered_map<std::string, int> sensorIndexMap;
    std::unordered_map<std::string, SensorMotorInfo> sensorInfoMap;
};

// Modern controller implementation with C++17 features
class BasicController final : public AbstractController {
private:
    // Use vector instead of raw arrays
    std::vector<double> weights;
    int sensornumber{0};
    int motornumber{0};
    
    // Structured bindings helper
    struct Dimensions {
        int sensors;
        int motors;
    };
    
public:
    // Constructor with inline initialization
    BasicController() 
        : AbstractController("BasicController", "$Id$") {}
    
    // Rule of 5 - explicitly defaulted
    ~BasicController() override = default;
    BasicController(const BasicController&) = delete;
    BasicController& operator=(const BasicController&) = delete;
    BasicController(BasicController&&) = default;
    BasicController& operator=(BasicController&&) = default;
    
    void init(int sensornumber, int motornumber, 
              std::optional<std::reference_wrapper<RandGen>> randGen = std::nullopt) override {
        this->sensornumber = sensornumber;
        this->motornumber = motornumber;
        
        // Use structured bindings
        const auto [rows, cols] = Dimensions{sensornumber, motornumber};
        
        // Resize vector (automatically handles memory)
        weights.resize(rows * cols);
        
        // Use STL algorithms with lambdas
        explicit if (randGen) {
            std::generate(weights.begin(), weights.end(), 
                         [&rng = randGen->get()]() { return rng.rand(); });
        } else {
            std::fill(weights.begin(), weights.end(), 0.1);
        }
    }
    
    [[nodiscard]] int getSensorNumber() const noexcept override {
        return sensornumber;
    }
    
    [[nodiscard]] int getMotorNumber() const noexcept override {
        return motornumber;
    }
    
#ifdef __cpp_lib_span
    void step(std::span<const sensor_t> sensors,
              std::span<motor_t> motors) override {
        // Modern implementation with ranges
        stepImpl(sensors, motors);
    }
#else
    void step(const sensor_t* sensors, int sensornumber,
              motor_t* motors, int motornumber) override {
        // Create spans manually for uniform interface
        stepImpl({sensors, static_cast<size_t>(sensornumber)},
                {motors, static_cast<size_t>(motornumber)});
    }
#endif
    
private:
    // Template implementation for both interfaces
    template<typename SensorRange, typename MotorRange>
    void stepImpl(SensorRange sensors, MotorRange motors) {
        // Use parallel execution policy if available
#ifdef __cpp_lib_execution
        std::fill(std::execution::par_unseq, motors.begin(), motors.end(), 0.0);
#else
        std::fill(motors.begin(), motors.end(), 0.0);
#endif
        
        // Matrix multiplication with modern C++
        for (size_t m = 0; m < motors.size(); ++m) {
            motors[m] = std::inner_product(
                sensors.begin(), sensors.end(),
                weights.begin() + m * sensors.size(),
                0.0
            );
        }
    }
    
public:
    // Use std::variant for flexible parameter types
    using Parameter = std::variant<double, int, std::string>;
    
    void setParameter(std::string_view name, Parameter value) {
        // Use std::visit for variant handling
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, double>) {
                // Handle double parameter
                if (name == "learning_rate") {
                    // Set learning rate
                }
            } else if constexpr (std::is_same_v<T, int>) {
                // Handle int parameter
            } else if constexpr (std::is_same_v<T, std::string>) {
                // Handle string parameter
            }
        }, value);
    }
};

// Factory function using auto return type
[[nodiscard]] inline auto createController(std::string_view type) 
    -> std::unique_ptr<AbstractController> {
    if (type == "basic") {
        return std::make_unique<BasicController>();
    }
    // Add more controller types here
    return nullptr;
}

// Example of using if constexpr for compile-time branching
template<typename Controller>
void configureController(Controller& ctrl) {
    if constexpr (std::is_base_of_v<BasicController, Controller>) {
        // Special configuration for BasicController
        ctrl.setParameter("learning_rate", 0.01);
    } else {
        // Generic configuration
    }
}

// ===== Additional Modern Features Example =====

// Use enum class instead of plain enum
enum class ControllerType : uint8_t {
    Basic,
    Advanced,
    Neural,
    Fuzzy
};

// Aggregate initialization with designated initializers (C++20)
struct ControllerConfig {
    int sensors{10};
    int motors{2};
    double learning_rate{0.01};
    bool adaptive{true};
    ControllerType type{ControllerType::Basic};
};

// Template variable for version info
template<typename T>
inline constexpr std::string_view controller_version = "1.0.0";

// Specialization for BasicController
template<>
inline constexpr std::string_view controller_version<BasicController> = "1.2.3";

#endif // MODERN_CONTROLLER_H