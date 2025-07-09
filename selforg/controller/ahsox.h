/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Adaptive Horizon Sox (AH-Sox) - A homeokinetic controller with       *
 *   dynamically adjustable prediction horizons                            *
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
#ifndef __AHSOX_H
#define __AHSOX_H

#include "abstractcontroller.h"
#include "controller_misc.h"
#include "configurable.h"
#include "matrix.h"
#include "circular_buffer.h"
#include <vector>
#include <deque>

/**
 * Adaptive Horizon Sox (AH-Sox)
 * 
 * This controller extends the Sox algorithm with dynamic prediction horizons.
 * It maintains multiple forward models with different time horizons and
 * automatically selects the most appropriate one based on prediction quality.
 * 
 * Key innovations:
 * - Multiple forward models (1-10 step predictions)
 * - Uncertainty estimation for each model
 * - Weighted combination based on prediction accuracy
 * - Automatic horizon selection for different behaviors
 * 
 * This allows the robot to handle both fast reflexive behaviors and
 * slower, more deliberate movements within a single framework.
 */
class AHSox : public AbstractController {
public:
    /// Configuration structure
    struct AHSoxConf {
        double initFeedbackStrength;  ///< initial strength of controller matrix C
        int minHorizon;               ///< minimum prediction horizon (default: 1)
        int maxHorizon;               ///< maximum prediction horizon (default: 10)
        double horizonAdaptRate;      ///< adaptation rate for horizon weights (default: 0.01)
        double uncertaintyThreshold;  ///< threshold for model uncertainty (default: 0.1)
        bool useWeightedCombination; ///< use weighted combination of models (default: true)
        
        double learningRateC;         ///< learning rate for controller
        double learningRateA;         ///< learning rate for forward models
        double creativity;            ///< creativity term
        double damping;              ///< damping/forgetting term
        bool useExtendedModel;       ///< use extended model with S matrix
        
        // Factors for learning rates
        double factorS;              ///< factor for S learning rate
        double factorb;              ///< factor for b learning rate
        double factorh;              ///< factor for h learning rate
    };
    
    /// Returns default configuration
    static AHSoxConf getDefaultConf() {
        AHSoxConf conf;
        conf.initFeedbackStrength = 1.0;
        conf.minHorizon = 1;
        conf.maxHorizon = 10;
        conf.horizonAdaptRate = 0.01;
        conf.uncertaintyThreshold = 0.1;
        conf.useWeightedCombination = true;
        conf.learningRateC = 0.1;
        conf.learningRateA = 0.1;
        conf.creativity = 0.0;
        conf.damping = 0.00001;
        conf.useExtendedModel = false;
        conf.factorS = 1.0;
        conf.factorb = 0.1;
        conf.factorh = 1.0;
        return conf;
    }
    
    /// Constructor
    AHSox(const AHSoxConf& conf = getDefaultConf());
    
    /// Destructor
    virtual ~AHSox();
    
    /// Initializes the controller
    virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr);
    
    /// Performs one step without learning
    virtual void stepNoLearning(const sensor* sensors, int number_sensors,
                               motor* motors, int number_motors);
    
    /// Performs one step with learning
    virtual void step(const sensor* sensors, int number_sensors,
                     motor* motors, int number_motors);
    
    /// Returns the number of sensors
    virtual int getSensorNumber() const override { return number_sensors; }
    
    /// Returns the number of motors  
    virtual int getMotorNumber() const override { return number_motors; }
    
    /// Stores the controller to file
    virtual bool store(FILE* f) const;
    
    /// Restores the controller from file
    virtual bool restore(FILE* f);
    
    /// Returns the current effective horizon
    int getCurrentHorizon() const { return static_cast<int>(currentHorizon); }
    
    /// Returns uncertainty for a specific horizon
    double getUncertainty(int horizon) const;
    
    /// Returns average uncertainty across all horizons
    double getAverageUncertainty() const;
    
protected:
    /// Forward model for a specific horizon
    struct ForwardModel {
        matrix::Matrix A;        ///< Forward model matrix
        matrix::Matrix S;        ///< Extended model (sensor branch)
        matrix::Matrix b;        ///< Model bias
        double uncertainty;      ///< Current uncertainty estimate
        double weight;           ///< Weight for combination
        std::deque<double> errorHistory; ///< Recent prediction errors
    };
    
    /// Calculates time-loop error for a specific horizon
    matrix::Matrix calculateTLE(int horizon) const;
    
    /// Updates uncertainty estimates for all models
    void updateUncertainties();
    
    /// Selects the best horizon based on current uncertainties
    int selectBestHorizon() const;
    
    /// Updates horizon weights based on prediction quality
    void updateHorizonWeights();
    
    /// Learns forward models for all horizons
    void learnModels();
    
    /// Learns controller using selected horizon(s)
    void learnController();
    
    /// Helper to get buffered values at specific time offset
    matrix::Matrix getBufferedSensor(int offset) const;
    matrix::Matrix getBufferedMotor(int offset) const;
    
protected:
    AHSoxConf conf;              ///< Configuration
    
    // Mutable parameter references for Configurable
    int minHorizon;
    int maxHorizon;
    double horizonAdaptRate;
    double uncertaintyThreshold;
    
    int number_sensors;          ///< Number of sensors
    int number_motors;           ///< Number of motors
    int buffersize;              ///< Size of history buffers
    int t;                       ///< Current timestep
    
    matrix::Matrix C;            ///< Controller matrix
    matrix::Matrix h;            ///< Controller bias
    matrix::Matrix L;            ///< Jacobian matrix
    
    std::vector<ForwardModel> models; ///< Forward models for each horizon
    paramval currentHorizon;     ///< Currently selected horizon
    
    // Buffers for sensor and motor values
    lpzrobots::CircularBuffer<matrix::Matrix, 50> x_buffer;
    lpzrobots::CircularBuffer<matrix::Matrix, 50> y_buffer;
    matrix::Matrix x_smooth;     ///< Smoothed sensor values
    
    // Additional parameters
    paramval epsC;               ///< Controller learning rate
    paramval epsA;               ///< Model learning rate
    paramval creativity;         ///< Creativity parameter
    paramval damping;            ///< Damping parameter
    paramval sense;              ///< Sensitivity parameter
    
    // Extended model activation functions
    static double g(double x) { return tanh(x); }
    static double g_s(double x) { double y = tanh(x); return 1.0 - y*y; }
    static double clip(double r, double x) { return x < -r ? -r : (x > r ? r : x); }
};

#endif // __AHSOX_H