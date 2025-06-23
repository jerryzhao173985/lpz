#!/usr/bin/env python3
"""Fix remaining uninitialized member variables in selforg."""

import os
import re

def fix_file(filepath, fixes):
    """Apply fixes to a specific file."""
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return False
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    modified = content
    for old_text, new_text in fixes:
        if old_text in modified:
            modified = modified.replace(old_text, new_text)
            print(f"  Fixed: {old_text[:50]}...")
        else:
            print(f"  Pattern not found: {old_text[:50]}...")
    
    if modified != content:
        with open(filepath, 'w') as f:
            f.write(modified)
        return True
    
    return False

def main():
    fixes = [
        # InvertNChannelController
        ("selforg/controller/invertnchannelcontroller.cpp", [
            ("InvertNChannelController::InvertNChannelController(int _buffersize, bool _update_only_1/*=false*/, bool inactivate_from_lowerbound)\n  : InvertController(\"InvertNChannelController\", \"$Id$\"),\n    buffersize(_buffersize), update_only_1(_update_only_1),\n    inactivate_from_lowerbound(inactivate_from_lowerbound)",
             "InvertNChannelController::InvertNChannelController(int _buffersize, bool _update_only_1/*=false*/, bool inactivate_from_lowerbound)\n  : InvertController(\"InvertNChannelController\", \"$Id$\"),\n    buffersize(_buffersize), update_only_1(_update_only_1),\n    inactivate_from_lowerbound(inactivate_from_lowerbound),\n    number_channels(0)")
        ]),
        
        # MutualInformationController
        ("selforg/controller/mutualinformationcontroller.cpp", [
            ("MutualInformationController::MutualInformationController(int measureMode, int historySize)\n    : AbstractController(\"MutualInformationController\", \"1.0\"),\n      measureMode(measureMode),\n      historySize(historySize)",
             "MutualInformationController::MutualInformationController(int measureMode, int historySize)\n    : AbstractController(\"MutualInformationController\", \"1.0\"),\n      measureMode(measureMode),\n      historySize(historySize),\n      sensorNumber(0),\n      motorNumber(0),\n      oldSensorStates(nullptr),\n      MI(0),\n      H_x(0),\n      H_yx(0),\n      H_Xsi(0),\n      ainit(0.1),\n      cinit(0.1)")
        ]),
        
        # NeuralGas
        ("selforg/controller/neuralgas.cpp", [
            ("NeuralGas::NeuralGas(double eps)\n  : AbstractModel(\"NeuralGas\", \"0.1\"),\n    eps(eps) {",
             "NeuralGas::NeuralGas(double eps)\n  : AbstractModel(\"NeuralGas\", \"0.1\"),\n    eps(eps),\n    maxTime(5000.0) {")
        ]),
        
        # OneActiveMultiPassiveController
        ("selforg/controller/oneactivemultipassivecontroller.cpp", [
            ("OneActiveMultiPassiveController::OneActiveMultiPassiveController( std::list<AbstractController*> controllers, const std::string& name, const std::string& revision)\n        : AbstractMultiController(controllers,name,revision),t(0),initialised(false) {",
             "OneActiveMultiPassiveController::OneActiveMultiPassiveController( std::list<AbstractController*> controllers, const std::string& name, const std::string& revision)\n        : AbstractMultiController(controllers,name,revision),t(0),initialised(false),passiveMotors(nullptr) {")
        ]),
        
        # OneControllerPerChannel
        ("selforg/controller/onecontrollerperchannel.cpp", [
            ("OneControllerPerChannel::OneControllerPerChannel(std::list<AbstractController*> controllers,\n                                                   const std::string& name, const std::string& revision)\n  : AbstractMultiController(controllers, name, revision)",
             "OneControllerPerChannel::OneControllerPerChannel(std::list<AbstractController*> controllers,\n                                                   const std::string& name, const std::string& revision)\n  : AbstractMultiController(controllers, name, revision),\n    motornumber(0),\n    sensornumber(0)")
        ]),
        
        # PiMax
        ("selforg/controller/pimax.cpp", [
            ("PiMax::PiMax()\n  : InvertController(\"PiMax\", \"$Id: pimax.cpp,v 1.2 2009/07/21 09:07:33 der Exp $\"),\n    conf(),\n    initialised(false)\n{",
             "PiMax::PiMax()\n  : InvertController(\"PiMax\", \"$Id: pimax.cpp,v 1.2 2009/07/21 09:07:33 der Exp $\"),\n    conf(),\n    initialised(false),\n    number_sensors(0),\n    number_motors(0)\n{")
        ]),
        
        # SineController
        ("selforg/controller/sinecontroller.cpp", [
            ("SineController::SineController(int controlmask, function func)\n  : AbstractController(\"SineController\", \"$Id$\"),\n    controlmask(controlmask), func(func) {",
             "SineController::SineController(int controlmask, function func)\n  : AbstractController(\"SineController\", \"$Id$\"),\n    controlmask(controlmask), func(func), individual(false) {"),
            
            ("MultiSineController::MultiSineController(int controlmask, function func)\n  : SineController(controlmask, func) {",
             "MultiSineController::MultiSineController(int controlmask, function func)\n  : SineController(controlmask, func),\n    periods(nullptr),\n    phaseShifts(nullptr),\n    amplitudes(nullptr) {")
        ])
    ]
    
    for filepath, file_fixes in fixes:
        print(f"\nProcessing: {filepath}")
        fix_file(filepath, file_fixes)

if __name__ == '__main__':
    main()