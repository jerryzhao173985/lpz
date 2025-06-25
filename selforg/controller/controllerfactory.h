/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/
#ifndef __CONTROLLERFACTORY_H
#define __CONTROLLERFACTORY_H

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include "abstractcontroller.h"

namespace lpzrobots {

/**
 * @brief Factory class for creating controller instances
 * 
 * This factory provides a centralized way to create controllers by type name,
 * supporting both built-in controllers and user-registered types.
 * 
 * @example
 * ```cpp
 * auto controller = ControllerFactory::createController("Sox");
 * if (controller) {
 *     controller->setParam("epsC", 0.1);
 *     controller->setParam("epsA", 0.05);
 * }
 * ```
 */
class ControllerFactory {
public:
    /// Function type for controller creators
    using CreatorFunc = std::function<std::unique_ptr<AbstractController>()>;
    
    /// Common controller configuration
    struct ControllerConfig {
        double learningRate = 0.1;
        double noiseLevel = 0.01;
        bool useTeaching = false;
        // Add more common parameters as needed
    };

    /**
     * @brief Creates a controller of the specified type
     * @param type The type name of the controller (e.g., "Sox", "DEP", "Pimax")
     * @return Unique pointer to the created controller, or nullptr if type not found
     */
    static std::unique_ptr<AbstractController> createController(const std::string& type);

    /**
     * @brief Creates a controller with configuration
     * @param type The type name of the controller
     * @param config Configuration parameters
     * @return Unique pointer to the created controller, or nullptr if type not found
     */
    static std::unique_ptr<AbstractController> createController(
        const std::string& type,
        const ControllerConfig& config);

    /**
     * @brief Creates a controller with custom parameters
     * @param type The type name of the controller
     * @param params Map of parameter name to value
     * @return Unique pointer to the created controller, or nullptr if type not found
     */
    static std::unique_ptr<AbstractController> createController(
        const std::string& type,
        const std::map<std::string, double>& params);

    /**
     * @brief Registers a new controller type
     * @param type The type name to register
     * @param creator Function that creates instances of this controller type
     * @return true if registration successful, false if type already exists
     */
    static bool registerControllerType(const std::string& type, CreatorFunc creator);

    /**
     * @brief Unregisters a controller type
     * @param type The type name to unregister
     * @return true if unregistration successful, false if type not found
     */
    static bool unregisterControllerType(const std::string& type);

    /**
     * @brief Gets a list of all registered controller types
     * @return Vector of registered type names
     */
    static std::vector<std::string> getRegisteredTypes();

    /**
     * @brief Checks if a controller type is registered
     * @param type The type name to check
     * @return true if type is registered
     */
    static bool isTypeRegistered(const std::string& type);

    /**
     * @brief Gets a brief description of a controller type
     * @param type The type name
     * @return Description string, or empty if type not found
     */
    static std::string getControllerDescription(const std::string& type);

    /**
     * @brief Registers all built-in controller types
     * 
     * This method is called automatically on first use, but can be
     * called manually to ensure all types are available.
     */
    static void registerBuiltinTypes();

    /**
     * @brief Gets controller categories for organization
     * @return Map of category name to list of controller types
     */
    static std::map<std::string, std::vector<std::string>> getControllersByCategory();

private:
    /// Map of controller type names to creator functions
    static std::unordered_map<std::string, CreatorFunc>& getCreatorMap();
    
    /// Map of controller type names to descriptions
    static std::unordered_map<std::string, std::string>& getDescriptionMap();
    
    /// Map of categories to controller types
    static std::map<std::string, std::vector<std::string>>& getCategoryMap();
    
    /// Ensures built-in types are registered
    static void ensureInitialized();
    
    /// Flag to track initialization
    static bool initialized;
    
    /// Helper to register a controller with description and category
    static void registerController(const std::string& type, 
                                 CreatorFunc creator,
                                 const std::string& description,
                                 const std::string& category);
};

/**
 * @brief RAII helper for temporary controller type registration
 * 
 * Useful for unit tests or temporary controller types.
 * 
 * @example
 * ```cpp
 * {
 *     ScopedControllerRegistration reg("TestController", 
 *         []() { return std::make_unique<TestController>(); });
 *     
 *     auto controller = ControllerFactory::createController("TestController");
 * } // TestController automatically unregistered here
 * ```
 */
class ScopedControllerRegistration {
public:
    ScopedControllerRegistration(const std::string& type, ControllerFactory::CreatorFunc creator)
        : type_(type), registered_(false) {
        registered_ = ControllerFactory::registerControllerType(type, creator);
    }
    
    ~ScopedControllerRegistration() {
        if (registered_) {
            ControllerFactory::unregisterControllerType(type_);
        }
    }
    
    // Non-copyable, non-movable
    ScopedControllerRegistration(const ScopedControllerRegistration&) = delete;
    ScopedControllerRegistration& operator=(const ScopedControllerRegistration&) = delete;
    ScopedControllerRegistration(ScopedControllerRegistration&&) = delete;
    ScopedControllerRegistration& operator=(ScopedControllerRegistration&&) = delete;
    
private:
    std::string type_;
    bool registered_;
};

} // namespace lpzrobots

#endif // __CONTROLLERFACTORY_H