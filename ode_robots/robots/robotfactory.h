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
#ifndef __ROBOTFACTORY_H
#define __ROBOTFACTORY_H

#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include "oderobot.h"

namespace lpzrobots {

// Forward declarations
class OdeHandle;
class OsgHandle;
class Primitive;

/**
 * @brief Factory class for creating robot instances
 * 
 * This factory provides a centralized way to create robots by type name,
 * supporting both built-in robots and user-registered types.
 * 
 * @example
 * ```cpp
 * auto robot = RobotFactory::createRobot("Sphererobot3Masses", odeHandle, osgHandle);
 * if (robot) {
 *     robot->place(Pos(0, 0, 0.5));
 * }
 * ```
 */
class RobotFactory {
public:
    /// Function type for robot creators
    using CreatorFunc = std::function<std::unique_ptr<OdeRobot>(
        const OdeHandle&, const OsgHandle&, const std::string&)>;
    
    /// Configuration structure for robot creation
    struct RobotConfig {
        std::string name = "Robot";
        double size = 1.0;
        double mass = 1.0;
        // Add more common configuration parameters as needed
    };

    /**
     * @brief Creates a robot of the specified type
     * @param type The type name of the robot (e.g., "Sphererobot3Masses")
     * @param odeHandle Handle to the ODE physics world
     * @param osgHandle Handle to the OSG graphics world
     * @param name Optional name for the robot instance
     * @return Unique pointer to the created robot, or nullptr if type not found
     */
    static std::unique_ptr<OdeRobot> createRobot(
        const std::string& type,
        const OdeHandle& odeHandle,
        const OsgHandle& osgHandle,
        const std::string& name = "");

    /**
     * @brief Creates a robot with configuration
     * @param type The type name of the robot
     * @param odeHandle Handle to the ODE physics world
     * @param osgHandle Handle to the OSG graphics world
     * @param config Configuration parameters
     * @return Unique pointer to the created robot, or nullptr if type not found
     */
    static std::unique_ptr<OdeRobot> createRobot(
        const std::string& type,
        const OdeHandle& odeHandle,
        const OsgHandle& osgHandle,
        const RobotConfig& config);

    /**
     * @brief Registers a new robot type
     * @param type The type name to register
     * @param creator Function that creates instances of this robot type
     * @return true if registration successful, false if type already exists
     */
    static bool registerRobotType(const std::string& type, CreatorFunc creator);

    /**
     * @brief Unregisters a robot type
     * @param type The type name to unregister
     * @return true if unregistration successful, false if type not found
     */
    static bool unregisterRobotType(const std::string& type);

    /**
     * @brief Gets a list of all registered robot types
     * @return Vector of registered type names
     */
    static std::vector<std::string> getRegisteredTypes();

    /**
     * @brief Checks if a robot type is registered
     * @param type The type name to check
     * @return true if type is registered
     */
    static bool isTypeRegistered(const std::string& type);

    /**
     * @brief Registers all built-in robot types
     * 
     * This method is called automatically on first use, but can be
     * called manually to ensure all types are available.
     */
    static void registerBuiltinTypes();

private:
    /// Map of robot type names to creator functions
    static std::unordered_map<std::string, CreatorFunc>& getCreatorMap();
    
    /// Ensures built-in types are registered
    static void ensureInitialized();
    
    /// Flag to track initialization
    static bool initialized;
};

/**
 * @brief RAII helper for temporary robot type registration
 * 
 * Useful for unit tests or temporary robot types.
 * 
 * @example
 * ```cpp
 * {
 *     ScopedRobotRegistration reg("TestRobot", 
 *         [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
 *             return std::make_unique<TestRobot>(ode, osg, name);
 *         });
 *     
 *     auto robot = RobotFactory::createRobot("TestRobot", odeHandle, osgHandle);
 * } // TestRobot automatically unregistered here
 * ```
 */
class ScopedRobotRegistration {
public:
    ScopedRobotRegistration(const std::string& type, RobotFactory::CreatorFunc creator)
        : type_(type), registered_(false) {
        registered_ = RobotFactory::registerRobotType(type, creator);
    }
    
    ~ScopedRobotRegistration() {
        if (registered_) {
            RobotFactory::unregisterRobotType(type_);
        }
    }
    
    // Non-copyable, non-movable
    ScopedRobotRegistration(const ScopedRobotRegistration&) = delete;
    ScopedRobotRegistration& operator=(const ScopedRobotRegistration&) = delete;
    ScopedRobotRegistration(ScopedRobotRegistration&&) = delete;
    ScopedRobotRegistration& operator=(ScopedRobotRegistration&&) = delete;
    
private:
    std::string type_;
    bool registered_;
};

} // namespace lpzrobots

#endif // __ROBOTFACTORY_H