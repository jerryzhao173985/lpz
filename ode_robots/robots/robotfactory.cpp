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

#include "robotfactory.h"
#include <iostream>
#include <algorithm>

// Include robot headers
#include "nimm2.h"
#include "nimm4.h"
#include "fourwheeled.h"
#include "sphererobot3masses.h"
#include "barrel2masses.h"
#include "axisorientationsensor.h"
#include "schlangeservo.h"
#include "schlangeservo2.h"
#include "uwo.h"
#include "hexapod.h"
#include "skeleton.h"
#include "odehandle.h"
#include "osghandle.h"

namespace lpzrobots {

// Static member initialization
bool RobotFactory::initialized = false;

std::unordered_map<std::string, RobotFactory::CreatorFunc>& 
RobotFactory::getCreatorMap() {
    static std::unordered_map<std::string, CreatorFunc> creators;
    return creators;
}

void RobotFactory::ensureInitialized() {
    if (!initialized) {
        registerBuiltinTypes();
        initialized = true;
    }
}

std::unique_ptr<OdeRobot> RobotFactory::createRobot(
    const std::string& type,
    const OdeHandle& odeHandle,
    const OsgHandle& osgHandle,
    const std::string& name) {
    
    ensureInitialized();
    
    auto& creators = getCreatorMap();
    auto it = creators.find(type);
    
    if (it != creators.end()) {
        return it->second(odeHandle, osgHandle, name.empty() ? type : name);
    }
    
    std::cerr << "RobotFactory: Unknown robot type '" << type << "'" << std::endl;
    std::cerr << "Available types: ";
    for (const auto& [typeName, _] : creators) {
        std::cerr << typeName << " ";
    }
    std::cerr << std::endl;
    
    return nullptr;
}

std::unique_ptr<OdeRobot> RobotFactory::createRobot(
    const std::string& type,
    const OdeHandle& odeHandle,
    const OsgHandle& osgHandle,
    const RobotConfig& config) {
    
    // For now, just use the name from config
    // In the future, this could pass the entire config to specialized creators
    return createRobot(type, odeHandle, osgHandle, config.name);
}

bool RobotFactory::registerRobotType(const std::string& type, CreatorFunc creator) {
    auto& creators = getCreatorMap();
    
    if (creators.find(type) != creators.end()) {
        std::cerr << "RobotFactory: Type '" << type << "' already registered" << std::endl;
        return false;
    }
    
    creators[type] = creator;
    return true;
}

bool RobotFactory::unregisterRobotType(const std::string& type) {
    auto& creators = getCreatorMap();
    return creators.erase(type) > 0;
}

std::vector<std::string> RobotFactory::getRegisteredTypes() {
    ensureInitialized();
    
    std::vector<std::string> types;
    const auto& creators = getCreatorMap();
    types.reserve(creators.size());
    
    for (const auto& [type, _] : creators) {
        types.push_back(type);
    }
    
    std::sort(types.begin(), types.end());
    return types;
}

bool RobotFactory::isTypeRegistered(const std::string& type) {
    ensureInitialized();
    const auto& creators = getCreatorMap();
    return creators.find(type) != creators.end();
}

void RobotFactory::registerBuiltinTypes() {
    // Register Nimm2 robot
    registerRobotType("Nimm2", 
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            return std::make_unique<Nimm2>(ode, osg, Nimm2::getDefaultConf(), name);
        });
    
    // Register Nimm4 robot
    registerRobotType("Nimm4",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            return std::make_unique<Nimm4>(ode, osg, name);
        });
    
    // Register FourWheeled robot
    registerRobotType("FourWheeled",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            FourWheeledConf conf = FourWheeled::getDefaultConf();
            return std::make_unique<FourWheeled>(ode, osg, conf, name);
        });
    
    // Register Sphererobot3Masses
    registerRobotType("Sphererobot3Masses",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            Sphererobot3MassesConf conf = Sphererobot3Masses::getDefaultConf();
            return std::make_unique<Sphererobot3Masses>(ode, osg, conf, name);
        });
    
    // Register Barrel2Masses
    registerRobotType("Barrel2Masses",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            Sphererobot3MassesConf conf = Sphererobot3Masses::getDefaultConf();
            return std::make_unique<Barrel2Masses>(ode, osg, conf, name);
        });
    
    // Register SchlangeServo
    registerRobotType("SchlangeServo",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            SchlangeConf conf = Schlange::getDefaultConf();
            conf.segmNumber = 10;  // Default number of segments
            return std::make_unique<SchlangeServo>(ode, osg, conf, name);
        });
    
    // Register SchlangeServo2
    registerRobotType("SchlangeServo2",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            SchlangeConf conf = Schlange::getDefaultConf();
            conf.segmNumber = 10;  // Default number of segments
            return std::make_unique<SchlangeServo2>(ode, osg, conf, name);
        });
    
    // Register UWO (Underactuated Wheeled Object)
    registerRobotType("UWO",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            UwoConf conf = Uwo::getDefaultConf();
            return std::make_unique<Uwo>(ode, osg, conf, name);
        });
    
    // Register Hexapod
    registerRobotType("Hexapod",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            HexapodConf conf = Hexapod::getDefaultConf();
            return std::make_unique<Hexapod>(ode, osg, conf, name);
        });
    
    // Register Skeleton
    registerRobotType("Skeleton",
        [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
            SkeletonConf conf = Skeleton::getDefaultConf();
            return std::make_unique<Skeleton>(ode, osg, conf, name);
        });
    
    // Add more built-in robots as needed
    // The pattern is:
    // registerRobotType("TypeName", 
    //     [](const OdeHandle& ode, const OsgHandle& osg, const std::string& name) {
    //         // Create and return the robot
    //         return std::make_unique<RobotClass>(...);
    //     });
}

} // namespace lpzrobots