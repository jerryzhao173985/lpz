# LPZRobots Framework Implementation Guide - Phase 1

## Quick Start Implementation

This guide provides concrete implementation steps for Phase 1 of the framework modernization, focusing on immediately usable improvements.

## 1. Logging Framework Implementation

### Step 1: Add selforg dependency to CMake

```cmake
# cmake/Dependencies.cmake - Add to existing file
FetchContent_Declare(
  selforg
  GIT_REPOSITORY https://github.com/gabime/selforg.git
  GIT_TAG        v1.13.0
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(selforg)
```

### Step 2: Create logging wrapper

```cpp
// selforg/utils/logger.h
#pragma once
#include <selforg/selforg.h>
#include <selforg/sinks/stdout_color_sinks.h>
#include <selforg/sinks/basic_file_sink.h>
#include <memory>

namespace lpzrobots {

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void init(const std::string& name = "lpzrobots") {
        // Create console sink with colors
        auto console_sink = std::make_shared<selforg::sinks::stdout_color_sink_mt>();
        console_sink->set_level(selforg::level::debug);
        
        // Create file sink
        auto file_sink = std::make_shared<selforg::sinks::basic_file_sink_mt>("lpzrobots.log", true);
        file_sink->set_level(selforg::level::trace);
        
        // Create logger with both sinks
        logger_ = std::make_shared<selforg::logger>(name, 
            selforg::sinks_init_list{console_sink, file_sink});
        
        // Set default level and pattern
        logger_->set_level(selforg::level::debug);
        logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
        
        // Register as default logger
        selforg::set_default_logger(logger_);
    }
    
    selforg::logger& get() { return *logger_; }
    
private:
    Logger() { init(); }
    std::shared_ptr<selforg::logger> logger_;
};

// Convenience macros with source location
#define LPZ_TRACE(...) selforg_TRACE(__VA_ARGS__)
#define LPZ_DEBUG(...) selforg_DEBUG(__VA_ARGS__)
#define LPZ_INFO(...)  selforg_INFO(__VA_ARGS__)
#define LPZ_WARN(...)  selforg_WARN(__VA_ARGS__)
#define LPZ_ERROR(...) selforg_ERROR(__VA_ARGS__)
#define LPZ_CRITICAL(...) selforg_CRITICAL(__VA_ARGS__)

} // namespace lpzrobots
```

### Step 3: Replace printf/cout in controllers

```cpp
// selforg/controller/sox.cpp - Example migration
void Sox::learn() {
    // OLD:
    // if (loga) printf("E=( ");
    
    // NEW:
    if (loga) {
        LPZ_DEBUG("Sox learning step t={}", t);
    }
    
    // ... learning code ...
    
    // OLD:
    // if (loga) printf(") = %f\n", E);
    
    // NEW:
    if (loga) {
        LPZ_DEBUG("Sox error E={:.6f}", E);
    }
}
```

## 2. Configuration Management Implementation

### Step 1: Add JSON/YAML dependencies

```cmake
# cmake/Dependencies.cmake
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        0.8.0
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(json yaml-cpp)
```

### Step 2: Create configuration class

```cpp
// selforg/utils/configuration.h
#pragma once
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <string>
#include <optional>
#include <fstream>

namespace lpzrobots {

class Configuration {
public:
    // Load from file (auto-detect format)
    bool loadFromFile(const std::string& filename) {
        if (filename.ends_with(".json")) {
            return loadFromJson(filename);
        } else if (filename.ends_with(".yaml") || filename.ends_with(".yml")) {
            return loadFromYaml(filename);
        }
        LPZ_ERROR("Unknown configuration format: {}", filename);
        return false;
    }
    
    // Type-safe getters
    template<typename T>
    T get(const std::string& path, const T& default_value = T{}) const {
        try {
            return getValueAtPath<T>(path);
        } catch (...) {
            return default_value;
        }
    }
    
    template<typename T>
    std::optional<T> getOptional(const std::string& path) const {
        try {
            return getValueAtPath<T>(path);
        } catch (...) {
            return std::nullopt;
        }
    }
    
    // Get sub-configuration
    Configuration getSection(const std::string& section) const {
        Configuration sub;
        if (data_.contains(section)) {
            sub.data_ = data_[section];
        }
        return sub;
    }
    
private:
    bool loadFromJson(const std::string& filename) {
        try {
            std::ifstream file(filename);
            file >> data_;
            return true;
        } catch (const std::exception& e) {
            LPZ_ERROR("Failed to load JSON config: {}", e.what());
            return false;
        }
    }
    
    bool loadFromYaml(const std::string& filename) {
        try {
            YAML::Node yaml = YAML::LoadFile(filename);
            data_ = yamlToJson(yaml);
            return true;
        } catch (const std::exception& e) {
            LPZ_ERROR("Failed to load YAML config: {}", e.what());
            return false;
        }
    }
    
    nlohmann::json yamlToJson(const YAML::Node& node) {
        // Convert YAML to JSON for unified handling
        nlohmann::json j;
        // ... conversion logic ...
        return j;
    }
    
    template<typename T>
    T getValueAtPath(const std::string& path) const {
        // Split path by '.'
        std::vector<std::string> parts;
        std::stringstream ss(path);
        std::string part;
        while (std::getline(ss, part, '.')) {
            parts.push_back(part);
        }
        
        // Navigate to value
        const nlohmann::json* current = &data_;
        for (const auto& p : parts) {
            if (current->is_object() && current->contains(p)) {
                current = &(*current)[p];
            } else {
                throw std::runtime_error("Path not found: " + path);
            }
        }
        
        return current->get<T>();
    }
    
    nlohmann::json data_;
};

// Global configuration instance
inline Configuration& getGlobalConfig() {
    static Configuration config;
    return config;
}

} // namespace lpzrobots
```

### Step 3: Use configuration in simulations

```cpp
// ode_robots/simulations/sphererobot_modern/main.cpp
class MySim : public Simulation {
public:
    MySim() {
        // Load configuration
        config_.loadFromFile("sphererobot.yaml");
        
        // Setup logging from config
        auto& logger = Logger::getInstance();
        logger.get().set_level(
            selforg::level::from_str(config_.get<std::string>("logging.level", "info")));
    }
    
    void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, 
               GlobalData& global) {
        LPZ_INFO("Starting sphererobot simulation");
        
        // Create robot from configuration
        auto robot_config = config_.getSection("robot");
        
        Sphererobot3MassesConf conf = Sphererobot3Masses::getDefaultConf();
        conf.radius = robot_config.get<double>("radius", 0.3);
        conf.pendulumMass = robot_config.get<double>("pendulum_mass", 0.2);
        
        auto* robot = new Sphererobot3Masses(odeHandle, osgHandle, conf, "Sphere1");
        robot->place(osg::Matrix::translate(0, 0, 0.5));
        
        // Create controller from configuration
        auto controller_config = config_.getSection("controller");
        std::string controller_type = controller_config.get<std::string>("type", "Sox");
        
        AbstractController* controller = nullptr;
        if (controller_type == "Sox") {
            SoxConf sox_conf = Sox::getDefaultConf();
            sox_conf.initFeedbackStrength = controller_config.get<double>("initFeedbackStrength", 1.0);
            sox_conf.learningRateC = controller_config.get<double>("epsC", 0.1);
            sox_conf.learningRateA = controller_config.get<double>("epsA", 0.05);
            controller = new Sox(sox_conf);
        }
        
        // Create agent
        auto* agent = new OdeAgent(global);
        agent->init(controller, robot, new One2OneWiring(new ColorUniformNoise(0.1)));
        global.agents.push_back(agent);
        global.configs.push_back(controller);
        
        LPZ_INFO("Created {} with {}", robot->getName(), controller_type);
    }
    
private:
    Configuration config_;
};
```

### Example configuration file

```yaml
# sphererobot.yaml
simulation:
  name: "Sphererobot Homeokinetic Control"
  description: "Self-organizing spherical robot with 3 masses"
  
logging:
  level: debug
  file: sphererobot.log
  
robot:
  radius: 0.3
  mass: 1.0
  pendulum_mass: 0.2
  pendulum_radius: 0.15
  
controller:
  type: Sox
  initFeedbackStrength: 1.0
  epsC: 0.1
  epsA: 0.05
  creativity: 0.1
  damping: 0.001
  
environment:
  gravity: -9.81
  ground_texture: "Images/greenground.rgb"
  sky_color: [0.1, 0.1, 0.6]
```

## 3. Error Handling Implementation

### Step 1: Create exception hierarchy

```cpp
// selforg/utils/exceptions.h
#pragma once
#include <exception>
#include <string>
#include <format>
#include <source_location>

namespace lpzrobots {

class LPZException : public std::exception {
public:
    LPZException(const std::string& message, 
                 const std::source_location& loc = std::source_location::current())
        : message_(message), location_(loc) {
        full_message_ = std::format("{} (at {}:{} in {})",
            message_,
            location_.file_name(),
            location_.line(),
            location_.function_name());
    }
    
    const char* what() const noexcept override {
        return full_message_.c_str();
    }
    
    const std::source_location& where() const noexcept {
        return location_;
    }
    
protected:
    std::string message_;
    std::source_location location_;
    std::string full_message_;
};

// Specific exceptions
class ConfigurationError : public LPZException {
    using LPZException::LPZException;
};

class ControllerError : public LPZException {
    using LPZException::LPZException;
};

class SimulationError : public LPZException {
    using LPZException::LPZException;
};

// Assertion replacement
#define LPZ_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            throw lpzrobots::LPZException( \
                std::format("Assertion failed: {} - {}", #condition, message)); \
        } \
    } while(0)

// Validation helpers
template<typename T>
void validateRange(const T& value, const T& min, const T& max, const std::string& name) {
    if (value < min || value > max) {
        throw LPZException(std::format("{} must be in range [{}, {}], got {}", 
                                      name, min, max, value));
    }
}

} // namespace lpzrobots
```

### Step 2: Update controller initialization

```cpp
// selforg/controller/sox.cpp
void Sox::init(int sensornumber, int motornumber, RandGen* randGen) {
    // Validate inputs
    LPZ_ASSERT(sensornumber > 0, "Sensor number must be positive");
    LPZ_ASSERT(motornumber > 0, "Motor number must be positive");
    LPZ_ASSERT(sensornumber >= motornumber, 
               "Sox requires at least as many sensors as motors");
    
    // Initialize base class
    ControllerBase::init(sensornumber, motornumber);
    
    // Validate configuration
    validateRange(conf.initFeedbackStrength, 0.0, 10.0, "initFeedbackStrength");
    validateRange(conf.learningRateC, 0.0, 1.0, "learningRateC");
    
    try {
        // Initialize matrices
        initModelMatrices(conf.initFeedbackStrength);
        initBiasVectors();
        initExtendedModel();
        
        // Initialize buffers
        x_buffer.init(buffersize, Matrix(number_sensors, 1));
        y_buffer.init(buffersize, Matrix(number_motors, 1));
        
        LPZ_INFO("Sox controller initialized: {} sensors, {} motors", 
                 sensornumber, motornumber);
                 
    } catch (const std::exception& e) {
        throw ControllerError(std::format("Failed to initialize Sox: {}", e.what()));
    }
}
```

## 4. Improved CMake Structure

### Step 1: Create core framework library

```cmake
# core/CMakeLists.txt
add_library(lpzrobots_core
  utils/logger.cpp
  utils/configuration.cpp
  utils/exceptions.cpp
)

target_link_libraries(lpzrobots_core
  PUBLIC
    selforg::selforg
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
)

target_include_directories(lpzrobots_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(lpzrobots_core PUBLIC cxx_std_20)

# Install headers
install(DIRECTORY utils/
  DESTINATION include/lpzrobots/core/utils
  FILES_MATCHING PATTERN "*.h"
)
```

### Step 2: Update component CMakeLists

```cmake
# selforg/CMakeLists.txt - Add core dependency
target_link_libraries(selforg
  PUBLIC
    lpzrobots::core  # Add this
    GSL::gsl
    ${CMAKE_DL_LIBS}
)
```

## 5. Integration Example

### Complete modernized controller

```cpp
// selforg/controller/sox_modern.cpp
#include "sox.h"
#include <lpzrobots/core/utils/logger.h>
#include <lpzrobots/core/utils/exceptions.h>
#include <lpzrobots/core/utils/configuration.h>

namespace lpzrobots {

void Sox::step(const sensor* x_, int number_sensors, 
               motor* y_, int number_motors) {
    LPZ_ASSERT(number_sensors == this->number_sensors, 
               "Sensor number mismatch");
    LPZ_ASSERT(number_motors == this->number_motors, 
               "Motor number mismatch");
    
    try {
        // Convert to matrices
        Matrix x(number_sensors, 1, x_);
        
        // Store sensor values
        x_buffer.push(x);
        
        // Compute motor values
        Matrix y = (C * x + h).map(g);
        
        // Add noise if configured
        if (y_noise_gen) {
            y += y_noise_gen->add(y);
        }
        
        // Store motor values
        y_buffer.push(y);
        
        // Learning step
        if (t >= buffersize) {
            learn();
        }
        
        // Convert back to array
        y.convertToBuffer(y_, number_motors);
        
        // Update time
        t++;
        
        // Log if verbose
        if (loga && t % 100 == 0) {
            LPZ_DEBUG("Sox step {}: avg_error={:.4f}, C_norm={:.4f}", 
                     t, calculateError(), C.norm());
        }
        
    } catch (const std::exception& e) {
        LPZ_ERROR("Error in Sox step: {}", e.what());
        // Fallback to safe motor values
        std::fill(y_, y_ + number_motors, 0.0);
    }
}

SoxConf Sox::getDefaultConf() {
    Configuration& config = getGlobalConfig();
    
    SoxConf conf;
    // Load from config file if available
    if (auto sox_config = config.getOptional<Configuration>("controllers.sox")) {
        conf.initFeedbackStrength = sox_config->get("initFeedbackStrength", 1.0);
        conf.learningRateC = sox_config->get("epsC", 0.1);
        conf.learningRateA = sox_config->get("epsA", 0.05);
        conf.creativity = sox_config->get("creativity", 0.0);
        conf.damping = sox_config->get("damping", 0.001);
        LPZ_INFO("Loaded Sox configuration from file");
    } else {
        // Use hardcoded defaults
        conf.initFeedbackStrength = 1.0;
        conf.learningRateC = 0.1;
        conf.learningRateA = 0.05;
        conf.creativity = 0.0;
        conf.damping = 0.001;
    }
    
    return conf;
}

} // namespace lpzrobots
```

## 6. Testing the New Framework

### Unit test example

```cpp
// tests/unit/framework/logger_test.cpp
#include <doctest/doctest.h>
#include <lpzrobots/core/utils/logger.h>
#include <sstream>

TEST_SUITE("Framework - Logger") {
    TEST_CASE("Logger initialization") {
        auto& logger = lpzrobots::Logger::getInstance();
        
        // Test logging at different levels
        LPZ_DEBUG("Debug message");
        LPZ_INFO("Info message");
        LPZ_WARN("Warning message");
        LPZ_ERROR("Error message");
        
        // Verify log file exists
        CHECK(std::filesystem::exists("lpzrobots.log"));
    }
    
    TEST_CASE("Logger formatting") {
        LPZ_INFO("Formatted number: {:.2f}", 3.14159);
        LPZ_INFO("Multiple args: {} + {} = {}", 2, 2, 4);
        
        // Check structured logging
        int sensor_count = 10;
        double error = 0.0234;
        LPZ_INFO("Controller state: sensors={}, error={:.4f}", sensor_count, error);
    }
}

TEST_SUITE("Framework - Configuration") {
    TEST_CASE("Load JSON configuration") {
        lpzrobots::Configuration config;
        
        // Create test config
        std::ofstream file("test_config.json");
        file << R"({
            "robot": {
                "type": "sphererobot",
                "radius": 0.3
            },
            "controller": {
                "type": "sox",
                "epsC": 0.1
            }
        })";
        file.close();
        
        CHECK(config.loadFromFile("test_config.json"));
        CHECK(config.get<std::string>("robot.type") == "sphererobot");
        CHECK(config.get<double>("robot.radius") == doctest::Approx(0.3));
        CHECK(config.get<double>("controller.epsC") == doctest::Approx(0.1));
        
        // Cleanup
        std::filesystem::remove("test_config.json");
    }
}

TEST_SUITE("Framework - Error Handling") {
    TEST_CASE("Exception with source location") {
        try {
            throw lpzrobots::ControllerError("Test error");
        } catch (const lpzrobots::LPZException& e) {
            CHECK(std::string(e.what()).find("Test error") != std::string::npos);
            CHECK(std::string(e.what()).find("framework/error_test.cpp") != std::string::npos);
        }
    }
    
    TEST_CASE("Validation helpers") {
        CHECK_THROWS_AS(
            lpzrobots::validateRange(1.5, 0.0, 1.0, "test_param"),
            lpzrobots::LPZException
        );
        
        CHECK_NOTHROW(
            lpzrobots::validateRange(0.5, 0.0, 1.0, "test_param")
        );
    }
}
```

## Next Steps

1. **Gradual Migration**: Start by adding logging to new code and critical paths
2. **Configuration Files**: Create YAML configs for existing simulations
3. **Error Handling**: Replace assertions with exceptions in controllers
4. **Documentation**: Update user guide with new configuration format

This Phase 1 implementation provides immediate benefits:

- ✅ Structured logging with levels and formatting
- ✅ Configuration file support (JSON/YAML)
- ✅ Better error messages with source locations
- ✅ Backward compatible with existing code
- ✅ Easy to integrate incrementally

Continue to Phase 2 for dependency injection and service architecture.
