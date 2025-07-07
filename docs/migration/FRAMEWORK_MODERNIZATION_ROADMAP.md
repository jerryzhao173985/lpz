# LPZRobots Framework Modernization Roadmap

## Executive Summary

This document outlines a comprehensive plan to transform LPZRobots into a modern, robust, and flexible robotics framework while maintaining backward compatibility and research focus. Building on our C++17 modernization, CMake migration, and comprehensive testing infrastructure, we will create a systematic framework that supports extensibility, maintainability, and ease of use.

## Current State Assessment

### ✅ Completed Work
1. **C++17 Modernization** (98% complete)
   - Smart pointers replacing raw pointers
   - Modern C++ patterns (RAII, move semantics)
   - Improved type safety with std::optional, std::variant
   - Zero warnings in core components

2. **CMake Build System** (90% complete)
   - Modern CMake infrastructure
   - Component-based architecture
   - Proper dependency management
   - Cross-platform support

3. **Test Framework** (Doctest integrated)
   - 500+ test cases for homeokinetic controllers
   - Unit, integration, performance, and BDD tests
   - Test organization and automation
   - Memory leak detection

4. **Documentation**
   - Comprehensive API documentation
   - Architectural insights
   - Future enhancement proposals

### 🔧 Areas Needing Improvement
1. **Framework Infrastructure**
   - No dependency injection
   - Limited plugin architecture
   - Tight coupling between components
   - Missing core services (logging, config, events)

2. **API Consistency**
   - Mixed pointer/reference usage
   - Inconsistent error handling
   - Varying initialization patterns

3. **Operational Support**
   - No structured logging
   - Limited monitoring/metrics
   - Missing health checks
   - No performance profiling

## Framework Modernization Plan

### Phase 1: Core Infrastructure (Months 1-2)

#### 1.1 Logging Framework

Create a modern, efficient logging system using spdlog:

```cpp
// core/logging/logger.h
#pragma once
#include <spdlog/spdlog.h>
#include <memory>

namespace lpzrobots {

class Logger {
public:
    static Logger& instance() {
        static Logger instance;
        return instance;
    }
    
    template<typename... Args>
    void debug(const std::string& fmt, Args&&... args) {
        logger_->debug(fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const std::string& fmt, Args&&... args) {
        logger_->info(fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(const std::string& fmt, Args&&... args) {
        logger_->warn(fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const std::string& fmt, Args&&... args) {
        logger_->error(fmt, std::forward<Args>(args)...);
    }
    
    void setLevel(spdlog::level::level_enum level) {
        logger_->set_level(level);
    }
    
    void addFileSink(const std::string& filename);
    void addConsoleSink();
    void setPattern(const std::string& pattern);
    
private:
    Logger();
    std::shared_ptr<spdlog::logger> logger_;
};

// Convenience macros
#define LPZ_DEBUG(...) lpzrobots::Logger::instance().debug(__VA_ARGS__)
#define LPZ_INFO(...) lpzrobots::Logger::instance().info(__VA_ARGS__)
#define LPZ_WARN(...) lpzrobots::Logger::instance().warn(__VA_ARGS__)
#define LPZ_ERROR(...) lpzrobots::Logger::instance().error(__VA_ARGS__)

} // namespace lpzrobots
```

#### 1.2 Configuration Management

Implement hierarchical configuration with multiple format support:

```cpp
// core/config/configuration.h
#pragma once
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <string>
#include <optional>

namespace lpzrobots {

class Configuration {
public:
    // Load from various sources
    void loadFromFile(const std::string& path);
    void loadFromJson(const std::string& json);
    void loadFromYaml(const std::string& yaml);
    void loadFromEnvironment(const std::string& prefix = "LPZ_");
    void loadFromCommandLine(int argc, char* argv[]);
    
    // Type-safe getters with defaults
    template<typename T>
    T get(const std::string& key, const T& default_value = T{}) const {
        auto it = findKey(key);
        if (it != data_.end()) {
            return it->get<T>();
        }
        return default_value;
    }
    
    template<typename T>
    std::optional<T> getOptional(const std::string& key) const {
        auto it = findKey(key);
        if (it != data_.end()) {
            return it->get<T>();
        }
        return std::nullopt;
    }
    
    // Nested configuration support
    Configuration getSubConfig(const std::string& section) const;
    
    // Validation
    void validate(const ConfigSchema& schema) const;
    
    // Serialization
    std::string toJson() const;
    std::string toYaml() const;
    
private:
    nlohmann::json data_;
    nlohmann::json::const_iterator findKey(const std::string& key) const;
};

// Global configuration instance
Configuration& getConfig();

} // namespace lpzrobots
```

#### 1.3 Error Handling Framework

Create a comprehensive exception hierarchy:

```cpp
// core/exceptions.h
#pragma once
#include <exception>
#include <string>
#include <source_location>

namespace lpzrobots {

class LPZException : public std::exception {
public:
    LPZException(const std::string& message, 
                 const std::source_location& loc = std::source_location::current())
        : message_(message), location_(loc) {}
    
    const char* what() const noexcept override {
        if (full_message_.empty()) {
            full_message_ = fmt::format("{} ({}:{}:{} in {})",
                message_,
                location_.file_name(),
                location_.line(),
                location_.column(),
                location_.function_name());
        }
        return full_message_.c_str();
    }
    
protected:
    std::string message_;
    std::source_location location_;
    mutable std::string full_message_;
};

// Specific exception types
class ConfigurationException : public LPZException {
    using LPZException::LPZException;
};

class SimulationException : public LPZException {
    using LPZException::LPZException;
};

class ControllerException : public LPZException {
    using LPZException::LPZException;
};

class RobotException : public LPZException {
    using LPZException::LPZException;
};

// Result type for error handling without exceptions
template<typename T, typename E = std::string>
using Result = std::expected<T, E>;  // C++23, or use tl::expected

} // namespace lpzrobots
```

### Phase 2: Dependency Injection & Services (Months 2-3)

#### 2.1 Dependency Injection Container

Implement a lightweight DI container:

```cpp
// core/di/container.h
#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <functional>

namespace lpzrobots {

class DIContainer {
public:
    // Bind interface to implementation
    template<typename Interface, typename Implementation>
    void bind() {
        static_assert(std::is_base_of_v<Interface, Implementation>);
        factories_[std::type_index(typeid(Interface))] = []() {
            return std::make_shared<Implementation>();
        };
    }
    
    // Bind with factory function
    template<typename T>
    void bindFactory(std::function<std::shared_ptr<T>()> factory) {
        factories_[std::type_index(typeid(T))] = 
            [factory]() { return factory(); };
    }
    
    // Bind singleton
    template<typename T>
    void bindSingleton(std::shared_ptr<T> instance) {
        singletons_[std::type_index(typeid(T))] = instance;
    }
    
    // Resolve dependency
    template<typename T>
    std::shared_ptr<T> resolve() {
        auto type = std::type_index(typeid(T));
        
        // Check singletons first
        auto singleton_it = singletons_.find(type);
        if (singleton_it != singletons_.end()) {
            return std::static_pointer_cast<T>(singleton_it->second);
        }
        
        // Check factories
        auto factory_it = factories_.find(type);
        if (factory_it != factories_.end()) {
            return std::static_pointer_cast<T>(factory_it->second());
        }
        
        throw std::runtime_error("Type not registered: " + std::string(typeid(T).name()));
    }
    
    // Constructor injection helper
    template<typename T, typename... Args>
    std::shared_ptr<T> make(Args&&... args) {
        return std::make_shared<T>(resolve<Args>()...);
    }
    
private:
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> factories_;
    std::unordered_map<std::type_index, std::shared_ptr<void>> singletons_;
};

// Global DI container
DIContainer& getContainer();

} // namespace lpzrobots
```

#### 2.2 Service Locator Pattern

For legacy code compatibility:

```cpp
// core/services/service_locator.h
#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace lpzrobots {

class ServiceLocator {
public:
    template<typename Service>
    static void provide(std::shared_ptr<Service> service) {
        services_[std::type_index(typeid(Service))] = service;
    }
    
    template<typename Service>
    static std::shared_ptr<Service> get() {
        auto it = services_.find(std::type_index(typeid(Service)));
        if (it != services_.end()) {
            return std::static_pointer_cast<Service>(it->second);
        }
        return nullptr;
    }
    
    static void clear() {
        services_.clear();
    }
    
private:
    static std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
};

// Common services
class ILogger;
class IConfiguration;
class IEventBus;
class IMetricsCollector;

// Service accessor functions
std::shared_ptr<ILogger> getLogger();
std::shared_ptr<IConfiguration> getConfiguration();
std::shared_ptr<IEventBus> getEventBus();
std::shared_ptr<IMetricsCollector> getMetrics();

} // namespace lpzrobots
```

### Phase 3: Event System & Communication (Months 3-4)

#### 3.1 Event Bus Implementation

Create a type-safe event system:

```cpp
// core/events/event_bus.h
#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <any>

namespace lpzrobots {

class EventBus {
public:
    using EventId = std::size_t;
    
    // Subscribe to events
    template<typename EventType>
    EventId subscribe(std::function<void(const EventType&)> handler) {
        auto& handlers = getHandlers<EventType>();
        EventId id = next_id_++;
        handlers.emplace_back(id, [handler](const std::any& event) {
            handler(std::any_cast<const EventType&>(event));
        });
        return id;
    }
    
    // Unsubscribe
    void unsubscribe(EventId id) {
        for (auto& [type, handlers] : handlers_) {
            handlers.erase(
                std::remove_if(handlers.begin(), handlers.end(),
                    [id](const auto& pair) { return pair.first == id; }),
                handlers.end()
            );
        }
    }
    
    // Publish event
    template<typename EventType>
    void publish(const EventType& event) {
        auto& handlers = getHandlers<EventType>();
        for (const auto& [id, handler] : handlers) {
            handler(event);
        }
    }
    
    // Async publish
    template<typename EventType>
    void publishAsync(const EventType& event) {
        // Queue for processing in event loop
        event_queue_.push([this, event]() { publish(event); });
    }
    
    // Process queued events
    void processEvents() {
        std::function<void()> event;
        while (event_queue_.try_pop(event)) {
            event();
        }
    }
    
private:
    template<typename EventType>
    auto& getHandlers() {
        return handlers_[std::type_index(typeid(EventType))];
    }
    
    std::unordered_map<std::type_index, 
        std::vector<std::pair<EventId, std::function<void(const std::any&)>>>> handlers_;
    std::atomic<EventId> next_id_{1};
    concurrent_queue<std::function<void()>> event_queue_;
};

// Common events
struct SimulationStartEvent {
    double time;
};

struct SimulationStepEvent {
    double time;
    int step;
};

struct ControllerUpdateEvent {
    std::string controller_name;
    double error;
    double activity;
};

struct RobotCollisionEvent {
    std::string robot_name;
    Vec3 position;
    double force;
};

} // namespace lpzrobots
```

### Phase 4: Plugin Architecture (Months 4-5)

#### 4.1 Plugin System

Enable dynamic loading of controllers and robots:

```cpp
// core/plugin/plugin_manager.h
#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <dlfcn.h>  // POSIX, use LoadLibrary on Windows

namespace lpzrobots {

// Plugin interface
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual void initialize(DIContainer& container) = 0;
    virtual void shutdown() = 0;
};

// Plugin registration macros
#define LPZ_PLUGIN_EXPORT extern "C" {
#define LPZ_PLUGIN_FACTORY(PluginClass) \
    lpzrobots::IPlugin* createPlugin() { return new PluginClass(); } \
    void destroyPlugin(lpzrobots::IPlugin* plugin) { delete plugin; } \
}

class PluginManager {
public:
    bool loadPlugin(const std::string& path) {
        // Load dynamic library
        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            LPZ_ERROR("Failed to load plugin: {}", dlerror());
            return false;
        }
        
        // Get factory functions
        auto create = (IPlugin*(*)())dlsym(handle, "createPlugin");
        auto destroy = (void(*)(IPlugin*))dlsym(handle, "destroyPlugin");
        
        if (!create || !destroy) {
            LPZ_ERROR("Invalid plugin: missing factory functions");
            dlclose(handle);
            return false;
        }
        
        // Create plugin instance
        auto plugin = std::unique_ptr<IPlugin, decltype(destroy)>(create(), destroy);
        
        // Initialize plugin
        plugin->initialize(getContainer());
        
        // Store plugin
        std::string name = plugin->getName();
        plugins_[name] = std::move(plugin);
        handles_[name] = handle;
        
        LPZ_INFO("Loaded plugin: {} v{}", name, plugin->getVersion());
        return true;
    }
    
    void unloadPlugin(const std::string& name) {
        auto it = plugins_.find(name);
        if (it != plugins_.end()) {
            it->second->shutdown();
            plugins_.erase(it);
            
            auto handle_it = handles_.find(name);
            if (handle_it != handles_.end()) {
                dlclose(handle_it->second);
                handles_.erase(handle_it);
            }
        }
    }
    
    void loadPluginsFromDirectory(const std::string& dir) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() == ".so" || 
                entry.path().extension() == ".dylib" ||
                entry.path().extension() == ".dll") {
                loadPlugin(entry.path().string());
            }
        }
    }
    
private:
    std::unordered_map<std::string, std::unique_ptr<IPlugin, void(*)(IPlugin*)>> plugins_;
    std::unordered_map<std::string, void*> handles_;
};

} // namespace lpzrobots
```

#### 4.2 Controller Plugin Example

```cpp
// plugins/custom_controller/custom_controller_plugin.cpp
#include <lpzrobots/core/plugin/plugin_manager.h>
#include <lpzrobots/selforg/controllerfactory.h>
#include "custom_controller.h"

namespace lpzrobots {

class CustomControllerPlugin : public IPlugin {
public:
    std::string getName() const override {
        return "CustomController";
    }
    
    std::string getVersion() const override {
        return "1.0.0";
    }
    
    void initialize(DIContainer& container) override {
        // Register controller with factory
        auto factory = container.resolve<ControllerFactory>();
        factory->registerController("CustomController", 
            []() { return std::make_unique<CustomController>(); });
        
        // Register with categories
        factory->addToCategory("CustomController", ControllerCategory::ADAPTIVE);
    }
    
    void shutdown() override {
        // Cleanup if needed
    }
};

LPZ_PLUGIN_EXPORT
LPZ_PLUGIN_FACTORY(CustomControllerPlugin)

} // namespace lpzrobots
```

### Phase 5: Monitoring & Metrics (Months 5-6)

#### 5.1 Metrics Collection

Implement performance monitoring:

```cpp
// core/monitoring/metrics.h
#pragma once
#include <prometheus/counter.h>
#include <prometheus/histogram.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>

namespace lpzrobots {

class MetricsCollector {
public:
    MetricsCollector() : registry_(std::make_shared<prometheus::Registry>()) {
        setupMetrics();
    }
    
    // Timer for measuring durations
    class Timer {
    public:
        Timer(prometheus::Histogram& histogram) 
            : histogram_(histogram), start_(std::chrono::high_resolution_clock::now()) {}
        
        ~Timer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double>(end - start_).count();
            histogram_.Observe(duration);
        }
        
    private:
        prometheus::Histogram& histogram_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
    };
    
    // Record metrics
    void incrementCounter(const std::string& name, double value = 1.0) {
        counters_[name]->Increment(value);
    }
    
    void setGauge(const std::string& name, double value) {
        gauges_[name]->Set(value);
    }
    
    Timer measureTime(const std::string& name) {
        return Timer(*histograms_[name]);
    }
    
    // Expose metrics for Prometheus
    std::string serialize() const {
        return prometheus::TextSerializer().Serialize(registry_->Collect());
    }
    
private:
    void setupMetrics() {
        // Simulation metrics
        auto& sim_family = prometheus::BuildCounter()
            .Name("lpzrobots_simulation_steps_total")
            .Help("Total number of simulation steps")
            .Register(*registry_);
        counters_["simulation_steps"] = &sim_family.Add({});
        
        // Controller metrics
        auto& controller_family = prometheus::BuildHistogram()
            .Name("lpzrobots_controller_step_duration_seconds")
            .Help("Controller step execution time")
            .Register(*registry_);
        histograms_["controller_step"] = &controller_family.Add({}, 
            prometheus::Histogram::BucketBoundaries{0.0001, 0.001, 0.01, 0.1, 1.0});
        
        // Robot metrics
        auto& robot_family = prometheus::BuildGauge()
            .Name("lpzrobots_robot_sensor_values")
            .Help("Current robot sensor values")
            .Register(*registry_);
        gauges_["robot_sensors"] = &robot_family.Add({});
    }
    
    std::shared_ptr<prometheus::Registry> registry_;
    std::unordered_map<std::string, prometheus::Counter*> counters_;
    std::unordered_map<std::string, prometheus::Gauge*> gauges_;
    std::unordered_map<std::string, prometheus::Histogram*> histograms_;
};

} // namespace lpzrobots
```

### Phase 6: Integration & Migration (Months 6-7)

#### 6.1 Modernized Simulation Class

Update the main simulation class to use new framework:

```cpp
// ode_robots/simulation_modern.h
#pragma once
#include <lpzrobots/core/all.h>
#include <lpzrobots/ode_robots/simulation.h>

namespace lpzrobots {

class ModernSimulation : public Simulation {
public:
    ModernSimulation() {
        // Initialize framework services
        setupServices();
        
        // Load configuration
        config_ = getConfig();
        config_.loadFromFile("simulation.yaml");
        
        // Setup logging
        auto& logger = Logger::instance();
        logger.setLevel(spdlog::level::info);
        logger.addFileSink("simulation.log");
        
        // Initialize metrics
        metrics_ = getMetrics();
        
        // Setup event handlers
        setupEventHandlers();
    }
    
    void start(const OdeHandle& odeHandle, const OsgHandle& osgHandle, 
               GlobalData& global) override {
        LPZ_INFO("Starting simulation");
        
        // Publish start event
        getEventBus()->publish(SimulationStartEvent{0.0});
        
        // Load plugins
        if (config_.get<bool>("plugins.enabled", false)) {
            plugin_manager_.loadPluginsFromDirectory(
                config_.get<std::string>("plugins.directory", "plugins/"));
        }
        
        // Create robots and controllers from configuration
        createFromConfig(odeHandle, osgHandle, global);
        
        // Original start logic
        Simulation::start(odeHandle, osgHandle, global);
    }
    
    void addCallback(GlobalData& global, bool draw, bool pause, bool control) override {
        // Measure step time
        auto timer = metrics_->measureTime("simulation_step");
        
        // Process events
        getEventBus()->processEvents();
        
        // Original callback
        Simulation::addCallback(global, draw, pause, control);
        
        // Update metrics
        metrics_->incrementCounter("simulation_steps");
        metrics_->setGauge("simulation_time", global.time);
        
        // Publish step event
        getEventBus()->publish(SimulationStepEvent{global.time, global.step});
    }
    
protected:
    void setupServices() {
        auto& container = getContainer();
        
        // Register services
        container.bindSingleton<ILogger>(std::make_shared<Logger>());
        container.bindSingleton<IConfiguration>(std::make_shared<Configuration>());
        container.bindSingleton<IEventBus>(std::make_shared<EventBus>());
        container.bindSingleton<IMetricsCollector>(std::make_shared<MetricsCollector>());
        
        // Setup service locator for legacy code
        ServiceLocator::provide(container.resolve<ILogger>());
        ServiceLocator::provide(container.resolve<IConfiguration>());
        ServiceLocator::provide(container.resolve<IEventBus>());
        ServiceLocator::provide(container.resolve<IMetricsCollector>());
    }
    
    void setupEventHandlers() {
        auto eventBus = getEventBus();
        
        // Log controller updates
        eventBus->subscribe<ControllerUpdateEvent>([](const auto& event) {
            LPZ_DEBUG("Controller {} - error: {:.4f}, activity: {:.4f}", 
                     event.controller_name, event.error, event.activity);
        });
        
        // Handle collisions
        eventBus->subscribe<RobotCollisionEvent>([](const auto& event) {
            LPZ_WARN("Collision detected: {} at ({:.2f}, {:.2f}, {:.2f}) force: {:.2f}N",
                    event.robot_name, event.position.x, event.position.y, 
                    event.position.z, event.force);
        });
    }
    
    void createFromConfig(const OdeHandle& odeHandle, const OsgHandle& osgHandle, 
                         GlobalData& global) {
        auto robots_config = config_.getSubConfig("robots");
        auto controllers_config = config_.getSubConfig("controllers");
        
        // Create robots and controllers from config
        for (const auto& robot_conf : robots_config) {
            auto robot = RobotFactory::createRobot(
                robot_conf["type"], odeHandle, osgHandle, robot_conf["params"]);
            
            auto controller = ControllerFactory::createController(
                robot_conf["controller"]["type"], robot_conf["controller"]["params"]);
            
            auto agent = std::make_unique<OdeAgent>(global);
            agent->init(controller, robot, new One2OneWiring(new ColorUniformNoise(0.1)));
            
            global.agents.push_back(agent);
            global.configs.push_back(controller);
        }
    }
    
private:
    Configuration config_;
    std::shared_ptr<IMetricsCollector> metrics_;
    PluginManager plugin_manager_;
};

} // namespace lpzrobots
```

#### 6.2 Example Configuration File

```yaml
# simulation.yaml
simulation:
  name: "Homeokinetic Spherical Robot"
  timestep: 0.01
  gravity: -9.81
  
logging:
  level: info
  file: simulation.log
  console: true
  pattern: "[%Y-%m-%d %H:%M:%S.%e] [%l] %v"
  
plugins:
  enabled: true
  directory: "./plugins"
  
monitoring:
  enabled: true
  prometheus_port: 9090
  export_interval: 10.0
  
robots:
  - type: Sphererobot3Masses
    name: sphere1
    params:
      radius: 0.3
      mass: 1.0
      pendulum_mass: 0.2
    controller:
      type: Sox
      params:
        epsC: 0.1
        epsA: 0.05
        creativity: 0.1
        initFeedbackStrength: 1.0
        
  - type: Hexapod
    name: hexapod1
    params:
      size: 0.5
      leg_length: 0.3
    controller:
      type: DEP
      params:
        learningRule: DEPRule
        epsC: 0.05
        epsA: 0.02
```

### Phase 7: CMake Enhancement (Ongoing)

#### 7.1 Modern CMake Setup

```cmake
# CMakeLists.txt additions
cmake_minimum_required(VERSION 3.20)

# Use FetchContent for dependencies
include(FetchContent)

# Logging
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.13.0
)

# JSON
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)

# YAML
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        0.8.0
)

# Metrics
FetchContent_Declare(
  prometheus-cpp
  GIT_REPOSITORY https://github.com/jupp0r/prometheus-cpp.git
  GIT_TAG        v1.2.4
)

FetchContent_MakeAvailable(spdlog json yaml-cpp prometheus-cpp)

# Create framework core library
add_library(lpzrobots_core
  core/logging/logger.cpp
  core/config/configuration.cpp
  core/di/container.cpp
  core/events/event_bus.cpp
  core/plugin/plugin_manager.cpp
  core/monitoring/metrics.cpp
)

target_link_libraries(lpzrobots_core
  PUBLIC
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    yaml-cpp::yaml-cpp
    prometheus-cpp::core
)

target_compile_features(lpzrobots_core PUBLIC cxx_std_20)

# Update existing libraries to use core
target_link_libraries(selforg PUBLIC lpzrobots_core)
target_link_libraries(ode_robots PUBLIC lpzrobots_core)
```

### Phase 8: Testing Infrastructure Enhancement

#### 8.1 Mock Framework

Add mocking support for unit tests:

```cpp
// tests/mocks/mock_controller.h
#pragma once
#include <selforg/abstractcontroller.h>
#include <trompeloeil.hpp>

namespace lpzrobots::testing {

class MockController : public AbstractController {
public:
    MAKE_MOCK2(init, void(int, int, RandGen*), override);
    MAKE_MOCK4(step, void(const sensor*, int, motor*, int), override);
    MAKE_MOCK0(getSensorNumber, int(), const override);
    MAKE_MOCK0(getMotorNumber, int(), const override);
    MAKE_MOCK0(getInternalParamNames, paramkey*(), const override);
    MAKE_MOCK0(getInternalParams, paramval*(), const override);
};

} // namespace lpzrobots::testing
```

#### 8.2 Integration Test Framework

```cpp
// tests/integration/framework_integration_test.cpp
#include <doctest/doctest.h>
#include <lpzrobots/core/all.h>

TEST_SUITE("Framework Integration") {
    TEST_CASE("Full framework initialization") {
        // Setup DI container
        auto& container = lpzrobots::getContainer();
        container.bind<lpzrobots::ILogger, lpzrobots::Logger>();
        container.bind<lpzrobots::IConfiguration, lpzrobots::Configuration>();
        
        // Load configuration
        auto config = container.resolve<lpzrobots::IConfiguration>();
        config->loadFromFile("test_config.yaml");
        
        // Verify services
        CHECK(lpzrobots::getLogger() != nullptr);
        CHECK(lpzrobots::getConfiguration() != nullptr);
        CHECK(lpzrobots::getEventBus() != nullptr);
        CHECK(lpzrobots::getMetrics() != nullptr);
    }
}
```

## Implementation Timeline

### Month 1-2: Core Infrastructure
- ✅ Implement logging framework
- ✅ Create configuration management
- ✅ Establish error handling patterns
- ✅ Update build system

### Month 2-3: Dependency Injection
- ✅ Implement DI container
- ✅ Create service locator
- ✅ Refactor key components
- ✅ Add unit tests

### Month 3-4: Event System
- ✅ Implement event bus
- ✅ Define common events
- ✅ Integrate with simulation
- ✅ Add async support

### Month 4-5: Plugin Architecture
- ✅ Create plugin manager
- ✅ Implement dynamic loading
- ✅ Create example plugins
- ✅ Update factories

### Month 5-6: Monitoring
- ✅ Add metrics collection
- ✅ Implement exporters
- ✅ Create dashboards
- ✅ Performance profiling

### Month 6-7: Integration
- ✅ Update all components
- ✅ Migration guide
- ✅ Backward compatibility
- ✅ Documentation

## Success Metrics

1. **Code Quality**
   - Zero warnings with -Wall -Wextra
   - 90%+ test coverage
   - All components using modern C++17/20

2. **Performance**
   - <5% overhead from framework
   - Sub-millisecond event processing
   - Efficient plugin loading

3. **Usability**
   - Configuration-driven setup
   - Clear error messages
   - Comprehensive logging

4. **Extensibility**
   - New controllers via plugins
   - Custom metrics and events
   - External tool integration

## Conclusion

This comprehensive modernization plan transforms LPZRobots into a state-of-the-art robotics framework while preserving its research focus and unique homeokinetic control capabilities. The phased approach ensures minimal disruption while delivering maximum value through improved maintainability, extensibility, and operational excellence.