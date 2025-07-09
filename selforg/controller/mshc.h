/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Multi-Scale Homeokinetic Controller (MSHC)                            *
 *   A hierarchical controller with multiple time scales                   *
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
#ifndef __MSHC_H
#define __MSHC_H

#include "abstractcontroller.h"
#include "controller_misc.h"
#include "configurable.h"
#include "matrix.h"
#include "circular_buffer.h"
#include <vector>
#include <deque>

/**
 * Multi-Scale Homeokinetic Controller (MSHC)
 * 
 * This controller implements hierarchical time-loop errors at multiple scales:
 * - Fast scale (1-5 steps): Reflexes and immediate stability
 * - Medium scale (10-50 steps): Basic behavioral patterns
 * - Slow scale (100+ steps): Long-term behavioral sequences
 * 
 * Key innovations:
 * - Separate forward models for each time scale
 * - Cross-scale coupling through shared prediction errors
 * - Scale-specific learning rates
 * - Emergent hierarchical behaviors through scale interaction
 * 
 * The controller naturally develops hierarchical behaviors where fast
 * scales handle stability while slow scales coordinate complex patterns.
 */
class MSHC : public AbstractController {
public:
    /// Time scale definition
    struct TimeScale {
        int horizon;                  ///< Prediction horizon for this scale
        double weight;                ///< Relative importance weight
        double learningRateFactor;    ///< Learning rate multiplier
        matrix::Matrix A;             ///< Forward model
        matrix::Matrix S;             ///< Extended model (sensor branch)
        matrix::Matrix b;             ///< Model bias
        matrix::Matrix C;             ///< Scale-specific controller contribution
        matrix::Matrix h;             ///< Scale-specific bias
        std::deque<double> errors;    ///< Recent prediction errors
        double activity;              ///< Current activity level
    };
    
    /// Configuration structure
    struct MSHCConf {
        double initFeedbackStrength;  ///< Initial controller strength
        bool useExtendedModel;        ///< Use extended model with S matrix
        
        // Time scale configuration
        std::vector<int> scaleHorizons;     ///< Horizons for each scale
        std::vector<double> scaleWeights;   ///< Initial weights for scales
        
        double crossScaleCoupling;    ///< Coupling strength between scales
        double scaleAdaptRate;        ///< Adaptation rate for scale weights
        bool dynamicScaleWeights;     ///< Adapt scale weights online
        
        // Learning parameters
        double learningRateC;         ///< Base learning rate for controller
        double learningRateA;         ///< Base learning rate for models
        double creativity;            ///< Creativity/exploration term
        double damping;              ///< Forgetting factor
        
        // Scale-specific factors
        double fastScaleFactor;       ///< Learning rate factor for fast scale
        double slowScaleFactor;       ///< Learning rate factor for slow scale
    };
    
    /// Returns default configuration
    static MSHCConf getDefaultConf() {
        MSHCConf conf;
        conf.initFeedbackStrength = 1.0;
        conf.useExtendedModel = false;
        
        // Default: 3 scales - fast, medium, slow
        conf.scaleHorizons = {2, 20, 100};
        conf.scaleWeights = {0.5, 0.3, 0.2};
        
        conf.crossScaleCoupling = 0.1;
        conf.scaleAdaptRate = 0.01;
        conf.dynamicScaleWeights = true;
        
        conf.learningRateC = 0.1;
        conf.learningRateA = 0.1;
        conf.creativity = 0.0;
        conf.damping = 0.00001;
        
        conf.fastScaleFactor = 2.0;   // Fast scale learns faster
        conf.slowScaleFactor = 0.5;   // Slow scale learns slower
        
        return conf;
    }
    
    /// Constructor
    MSHC(const MSHCConf& conf = getDefaultConf());
    
    /// Destructor
    virtual ~MSHC();
    
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
    
    /// Get activity level for a specific scale
    double getScaleActivity(int scaleIndex) const;
    
    /// Get current weight for a specific scale
    double getScaleWeight(int scaleIndex) const;
    
    /// Get weights for all scales
    std::vector<double> getScaleWeights() const;
    
protected:
    /// Calculate time-loop error for a specific scale
    matrix::Matrix calculateScaleTLE(const TimeScale& scale) const;
    
    /// Update scale activities based on recent errors
    void updateScaleActivities();
    
    /// Update scale weights based on activities
    void updateScaleWeights();
    
    /// Learn forward models for all scales
    void learnModels();
    
    /// Learn scale-specific controllers
    void learnControllers();
    
    /// Combine scale outputs with cross-scale coupling
    matrix::Matrix combineScaleOutputs(const matrix::Matrix& x) const;
    
    /// Apply cross-scale coupling to modify learning
    void applyCrossScaleCoupling();
    
    /// Helper functions for buffer access
    matrix::Matrix getBufferedSensor(int offset) const;
    matrix::Matrix getBufferedMotor(int offset) const;

public:
    /// Get current scale activities
    std::vector<double> getScaleActivities() const {
        std::vector<double> activities;
        for (const TimeScale& scale : scales) {
            activities.push_back(scale.activity);
        }
        return activities;
    }
    
    
protected:
    MSHCConf conf;                ///< Configuration
    
    // Mutable parameter references for Configurable
    double crossScaleCoupling;
    double scaleAdaptRate;
    
    int number_sensors;           ///< Number of sensors
    int number_motors;            ///< Number of motors
    int buffersize;               ///< Size of history buffers
    int t;                        ///< Current timestep
    
    std::vector<TimeScale> scales; ///< Time scales
    
    matrix::Matrix C_combined;     ///< Combined controller matrix
    matrix::Matrix h_combined;     ///< Combined controller bias
    
    // Buffers for sensor and motor values
    lpzrobots::CircularBuffer<matrix::Matrix, 150> x_buffer;
    lpzrobots::CircularBuffer<matrix::Matrix, 150> y_buffer;
    matrix::Matrix x_smooth;       ///< Smoothed sensor values
    
    // Learning parameters
    paramval epsC;                ///< Controller learning rate
    paramval epsA;                ///< Model learning rate
    paramval creativity;          ///< Creativity parameter
    paramval damping;             ///< Damping parameter
    
    // Activation functions
    static double g(double x) { return tanh(x); }
    static double g_s(double x) { double y = tanh(x); return 1.0 - y*y; }
    static double clip(double r, double x) { return x < -r ? -r : (x > r ? r : x); }
};

#endif // __MSHC_H