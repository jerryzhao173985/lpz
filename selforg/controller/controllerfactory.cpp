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

#include "controllerfactory.h"
#include <iostream>
#include <algorithm>

// Include controller headers
#include "sox.h"
#include "soxexpand.h"
#include "sos.h"
#include "dep.h"
#include "pimax.h"
#include "dercontroller.h"
#include "derinf.h"
#include "derlininvert.h"
#include "derbigcontroller.h"
#include "derpseudosensor.h"
#include "invertmotornstep.h"
#include "invertmotorspace.h"
#include "invertnchannelcontroller.h"
#include "sinecontroller.h"
#include "ffnncontroller.h"
#include "elman.h"
#include "onecontrollerperchannel.h"
// #include "selectiveone2onewiring.h"  // This is a wiring, not a controller
// #include "som.h"  // Need to check if these exist
// #include "soml.h"
// #include "semox.h"
// #include "classicreinforce.h"
// #include "qlearning.h"

namespace lpzrobots {

// Static member initialization
bool ControllerFactory::initialized = false;

std::unordered_map<std::string, ControllerFactory::CreatorFunc>& 
ControllerFactory::getCreatorMap() {
    static std::unordered_map<std::string, CreatorFunc> creators;
    return creators;
}

std::unordered_map<std::string, std::string>& 
ControllerFactory::getDescriptionMap() {
    static std::unordered_map<std::string, std::string> descriptions;
    return descriptions;
}

std::map<std::string, std::vector<std::string>>& 
ControllerFactory::getCategoryMap() {
    static std::map<std::string, std::vector<std::string>> categories;
    return categories;
}

void ControllerFactory::ensureInitialized() {
    if (!initialized) {
        registerBuiltinTypes();
        initialized = true;
    }
}

std::unique_ptr<AbstractController> ControllerFactory::createController(const std::string& type) {
    ensureInitialized();
    
    auto& creators = getCreatorMap();
    auto it = creators.find(type);
    
    if (it != creators.end()) {
        return it->second();
    }
    
    std::cerr << "ControllerFactory: Unknown controller type '" << type << "'" << std::endl;
    std::cerr << "Available types: ";
    for (const auto& [typeName, _] : creators) {
        std::cerr << typeName << " ";
    }
    std::cerr << std::endl;
    
    return nullptr;
}

std::unique_ptr<AbstractController> ControllerFactory::createController(
    const std::string& type,
    const ControllerConfig& config) {
    
    auto controller = createController(type);
    if (controller) {
        // Apply common configuration
        if (controller->hasParam("eps")) {
            controller->setParam("eps", config.learningRate);
        }
        if (controller->hasParam("noise")) {
            controller->setParam("noise", config.noiseLevel);
        }
        // Add more parameter mappings as needed
    }
    return controller;
}

std::unique_ptr<AbstractController> ControllerFactory::createController(
    const std::string& type,
    const std::map<std::string, double>& params) {
    
    auto controller = createController(type);
    if (controller) {
        for (const auto& [param, value] : params) {
            if (controller->hasParam(param)) {
                controller->setParam(param, value);
            }
        }
    }
    return controller;
}

bool ControllerFactory::registerControllerType(const std::string& type, CreatorFunc creator) {
    auto& creators = getCreatorMap();
    
    if (creators.find(type) != creators.end()) {
        std::cerr << "ControllerFactory: Type '" << type << "' already registered" << std::endl;
        return false;
    }
    
    creators[type] = creator;
    return true;
}

void ControllerFactory::registerController(const std::string& type, 
                                         CreatorFunc creator,
                                         const std::string& description,
                                         const std::string& category) {
    registerControllerType(type, creator);
    getDescriptionMap()[type] = description;
    getCategoryMap()[category].push_back(type);
}

bool ControllerFactory::unregisterControllerType(const std::string& type) {
    auto& creators = getCreatorMap();
    auto& descriptions = getDescriptionMap();
    auto& categories = getCategoryMap();
    
    // Remove from categories
    for (auto& [cat, types] : categories) {
        types.erase(std::remove(types.begin(), types.end(), type), types.end());
    }
    
    descriptions.erase(type);
    return creators.erase(type) > 0;
}

std::vector<std::string> ControllerFactory::getRegisteredTypes() {
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

bool ControllerFactory::isTypeRegistered(const std::string& type) {
    ensureInitialized();
    const auto& creators = getCreatorMap();
    return creators.find(type) != creators.end();
}

std::string ControllerFactory::getControllerDescription(const std::string& type) {
    ensureInitialized();
    const auto& descriptions = getDescriptionMap();
    auto it = descriptions.find(type);
    return (it != descriptions.end()) ? it->second : "";
}

std::map<std::string, std::vector<std::string>> ControllerFactory::getControllersByCategory() {
    ensureInitialized();
    return getCategoryMap();
}

void ControllerFactory::registerBuiltinTypes() {
    // Homeokinetic Controllers
    registerController("Sox", 
        []() { return std::make_unique<Sox>(); },
        "Self-organizing controller with extended world model",
        "Homeokinetic");
    
    registerController("SoxExpand", 
        []() { return std::make_unique<SoxExpand>(); },
        "Sox with context integration",
        "Homeokinetic");
    
    registerController("SoS", 
        []() { return std::make_unique<Sos>(); },
        "Self-organizing controller with Hebb-like learning",
        "Homeokinetic");
    
    registerController("DEP", 
        []() { return std::make_unique<DEP>(); },
        "Differential Extrinsic Plasticity controller",
        "Homeokinetic");
    
    registerController("Pimax", 
        []() { return std::make_unique<PiMax>(); },
        "Predictive information maximization controller",
        "Homeokinetic");
    
    // registerController("Semox", 
    //     []() { return std::make_unique<Semox>(); },
    //     "Sensorimotor dynamics with extended world model",
    //     "Homeokinetic");
    
    // Derivative-based Controllers
    registerController("DerController", 
        []() { return std::make_unique<DerController>(); },
        "Basic derivative controller",
        "Derivative");
    
    registerController("DerInf", 
        []() { return std::make_unique<DerInf>(); },
        "Derivative controller with information maximization",
        "Derivative");
    
    registerController("DerLinInvert", 
        []() { return std::make_unique<DerLinInvert>(); },
        "Linear inversion with derivative sensors",
        "Derivative");
    
    registerController("DerBigController", 
        []() { return std::make_unique<DerBigController>(); },
        "Extended derivative controller for many DOF",
        "Derivative");
    
    // registerController("DerPseudoSensor", 
    //     []() { return std::make_unique<DerPseudoSensor<2, 2>>(); },
    //     "Derivative controller with pseudo sensors",
    //     "Derivative");
    
    // Motor Space Controllers
    registerController("InvertMotorNStep", 
        []() { return std::make_unique<InvertMotorNStep>(); },
        "N-step motor space controller",
        "Motor Space");
    
    registerController("InvertMotorSpace", 
        []() { return std::make_unique<InvertMotorSpace>(10, 0.1); },
        "Basic motor space controller",
        "Motor Space");
    
    registerController("InvertNChannelController", 
        []() { return std::make_unique<InvertNChannelController>(10); },
        "N-channel inversion controller",
        "Motor Space");
    
    // Neural Network Controllers
    // registerController("FFNNController", 
    //     []() { return std::make_unique<FFNNController>(); },
    //     "Feed-forward neural network controller",
    //     "Neural Network");
    
    // registerController("Elman", 
    //     []() { return std::make_unique<Elman>(); },
    //     "Elman recurrent neural network",
    //     "Neural Network");
    
    // Self-Organizing Maps
    // registerController("SOM", 
    //     []() { return std::make_unique<Som>(); },
    //     "Self-organizing map controller",
    //     "Self-Organizing");
    
    // registerController("SOML", 
    //     []() { return std::make_unique<Soml>(); },
    //     "Self-organizing map with learning",
    //     "Self-Organizing");
    
    // Reinforcement Learning
    // registerController("ClassicReinforce", 
    //     []() { return std::make_unique<ClassicReinforce>(); },
    //     "Classical reinforcement learning",
    //     "Reinforcement");
    
    // registerController("QLearning", 
    //     []() { return std::make_unique<QLearning>(); },
    //     "Q-Learning controller",
    //     "Reinforcement");
    
    // Simple Controllers
    registerController("SineController", 
        []() { return std::make_unique<SineController>(); },
        "Simple sine wave generator",
        "Simple");
    
    // registerController("OneControllerPerChannel", 
    //     []() { return std::make_unique<OneControllerPerChannel>(); },
    //     "Separate controller for each channel",
    //     "Modular");
}

} // namespace lpzrobots