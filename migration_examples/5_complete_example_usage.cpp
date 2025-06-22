// Complete Example: Using All Migration Components Together
// This shows how to integrate the modernized components in a real application

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

// Include the modernized controller
#include "modern_controller.h"

// Platform-specific includes handled by CMake definitions
#ifdef MACOS_ARM64
    #include <TargetConditionals.h>
    #define PLATFORM_NAME "macOS ARM64 (Apple Silicon)"
#elif defined(MACOS_X86_64)
    #define PLATFORM_NAME "macOS x86_64 (Intel)"
#elif defined(LINUX_X86_64)
    #define PLATFORM_NAME "Linux x86_64"
#elif defined(LINUX_ARM64)
    #define PLATFORM_NAME "Linux ARM64"
#elif defined(WINDOWS)
    #define PLATFORM_NAME "Windows"
#else
    #define PLATFORM_NAME "Unknown Platform"
#endif

// Example of a modern simulation class using the new components
class ModernSimulation {
private:
    std::unique_ptr<AbstractController> controller;
    std::vector<sensor_t> sensors;
    std::vector<motor_t> motors;
    
    // Timing using std::chrono
    using clock = std::chrono::high_resolution_clock;
    using duration = std::chrono::duration<double>;
    
    clock::time_point startTime;
    size_t stepCount{0};
    
public:
    ModernSimulation(int sensorNum, int motorNum) 
        : sensors(sensorNum, 0.0)
        , motors(motorNum, 0.0) {
        
        std::cout << "Initializing simulation on " << PLATFORM_NAME << "\n";
        
        // Create controller using factory
        controller = createController("basic");
        if (!controller) {
            throw std::runtime_error("Failed to create controller");
        }
        
        // Initialize with optional random generator
        auto randGen = std::make_unique<RandGen>();
        controller->init(sensorNum, motorNum, std::ref(*randGen));
        
        startTime = clock::now();
    }
    
    void step() {
        // Simulate sensor readings
        auto now = clock::now();
        duration elapsed = now - startTime;
        
        // Generate some example sensor data
        for (size_t i = 0; i < sensors.size(); ++i) {
            sensors[i] = std::sin(elapsed.count() + i * 0.5);
        }
        
        // Controller step using modern interface
#ifdef __cpp_lib_span
        controller->step(sensors, motors);
#else
        controller->step(sensors.data(), sensors.size(), 
                        motors.data(), motors.size());
#endif
        
        stepCount++;
    }
    
    void printStatus() const {
        auto now = clock::now();
        duration elapsed = now - startTime;
        
        std::cout << "\nSimulation Status:\n";
        std::cout << "  Platform: " << PLATFORM_NAME << "\n";
        std::cout << "  Steps: " << stepCount << "\n";
        std::cout << "  Time: " << elapsed.count() << " seconds\n";
        std::cout << "  Steps/sec: " << stepCount / elapsed.count() << "\n";
        
        std::cout << "  Current motor values: ";
        for (const auto& m : motors) {
            std::cout << m << " ";
        }
        std::cout << "\n";
    }
    
    // Example of using structured bindings with custom return type
    struct PerformanceMetrics {
        double avgStepTime;
        double totalTime;
        size_t totalSteps;
    };
    
    [[nodiscard]] PerformanceMetrics getPerformanceMetrics() const {
        auto now = clock::now();
        duration elapsed = now - startTime;
        
        return {
            .avgStepTime = elapsed.count() / stepCount,
            .totalTime = elapsed.count(),
            .totalSteps = stepCount
        };
    }
};

// Example CMakeLists.txt snippet for building this:
/*
cmake_minimum_required(VERSION 3.16)
project(lpzrobots_example)

# Include platform detection
include(cmake/platform_detection.cmake)

# Find required packages
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

# Create executable
add_executable(simulation_example
    migration_examples/5_complete_example_usage.cpp
    # ... other source files
)

# Set platform-specific flags
set_platform_flags(simulation_example)

# Link libraries
target_link_libraries(simulation_example PRIVATE
    selforg
    Qt5::Core
    Qt5::Widgets
)

# Print platform info during configuration
print_platform_info()
*/

// Main function demonstrating usage
int main() {
    try {
        // Print build configuration
        std::cout << "lpzrobots Modern C++17 Example\n";
        std::cout << "==============================\n";
        std::cout << "Compiled for: " << PLATFORM_NAME << "\n";
        std::cout << "C++ Standard: " << __cplusplus << "\n";
        
#ifdef __cpp_lib_span
        std::cout << "Using C++20 span feature\n";
#else
        std::cout << "Using C++17 compatibility mode\n";
#endif
        
        // Create and run simulation
        ModernSimulation sim(10, 2);  // 10 sensors, 2 motors
        
        // Run for a few seconds
        auto runDuration = std::chrono::seconds(3);
        auto endTime = std::chrono::steady_clock::now() + runDuration;
        
        while (std::chrono::steady_clock::now() < endTime) {
            sim.step();
            
            // Simulate real-time constraints
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        // Print final status
        sim.printStatus();
        
        // Use structured bindings to get metrics
        auto [avgTime, totalTime, steps] = sim.getPerformanceMetrics();
        std::cout << "\nPerformance Summary:\n";
        std::cout << "  Average step time: " << avgTime * 1000 << " ms\n";
        std::cout << "  Total simulation time: " << totalTime << " seconds\n";
        std::cout << "  Total steps: " << steps << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

// Build instructions:
/*
# Configure with CMake
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --parallel

# Run
./simulation_example
*/