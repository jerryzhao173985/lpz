/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Context-Aware Sox (CA-Sox)                                            *
 *   A homeokinetic controller with context-dependent models               *
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
#ifndef __CASOX_H
#define __CASOX_H

#include "abstractcontroller.h"
#include "controller_misc.h"
#include "configurable.h"
#include "matrix.h"
#include "circular_buffer.h"
#include <vector>
#include <deque>

/**
 * Context-Aware Sox (CA-Sox)
 * 
 * This controller extends Sox with multiple context-dependent forward models.
 * It automatically detects different contexts (e.g., collision, free space,
 * interaction with objects) and switches between specialized models.
 * 
 * Key innovations:
 * - Mixture of experts architecture for forward models
 * - Online context detection through sensor clustering
 * - Smooth transitions between models using gating network
 * - Context-specific learning rates and exploration
 * 
 * The controller learns to recognize different situations and apply
 * appropriate control strategies for each context.
 */
class CASox : public AbstractController {
public:
    /// Context model structure
    struct ContextModel {
        // Model parameters
        matrix::Matrix A;              ///< Forward model
        matrix::Matrix S;              ///< Extended model (sensor branch)
        matrix::Matrix b;              ///< Model bias
        
        // Context detection
        matrix::Matrix prototype;      ///< Sensor prototype for this context
        double radius;                 ///< Context radius
        double activation;             ///< Current activation level
        std::deque<double> recentErrors; ///< Recent prediction errors
        
        // Statistics
        int useCount;                  ///< How often this context is active
        double avgError;               ///< Average prediction error
        
        // Context-specific parameters
        double explorationLevel;       ///< Exploration for this context
        double learningRateFactor;     ///< Learning rate modifier
    };
    
    /// Configuration structure
    struct CASoxConf {
        double initFeedbackStrength;   ///< Initial controller strength
        bool useExtendedModel;         ///< Use extended model with S
        
        // Context detection
        int maxContexts;               ///< Maximum number of contexts
        double contextRadius;          ///< Initial radius for contexts
        double contextThreshold;       ///< Threshold for creating new context
        double mergeThreshold;         ///< Threshold for merging contexts
        bool dynamicContexts;          ///< Allow dynamic context creation
        
        // Learning parameters
        double learningRateC;          ///< Controller learning rate
        double learningRateA;          ///< Model learning rate
        double gateLearningRate;       ///< Gating network learning rate
        double creativity;             ///< Creativity/exploration
        double damping;               ///< Forgetting factor
        
        // Context adaptation
        double contextAdaptRate;       ///< Rate of context prototype adaptation
        double radiusAdaptRate;        ///< Rate of radius adaptation
        bool contextSpecificExploration; ///< Different exploration per context
    };
    
    /// Returns default configuration
    static CASoxConf getDefaultConf() {
        CASoxConf conf;
        conf.initFeedbackStrength = 1.0;
        conf.useExtendedModel = false;
        
        conf.maxContexts = 5;
        conf.contextRadius = 0.5;
        conf.contextThreshold = 0.3;
        conf.mergeThreshold = 0.1;
        conf.dynamicContexts = true;
        
        conf.learningRateC = 0.1;
        conf.learningRateA = 0.1;
        conf.gateLearningRate = 0.05;
        conf.creativity = 0.0;
        conf.damping = 0.00001;
        
        conf.contextAdaptRate = 0.01;
        conf.radiusAdaptRate = 0.001;
        conf.contextSpecificExploration = true;
        
        return conf;
    }
    
    /// Constructor
    CASox(const CASoxConf& conf = getDefaultConf());
    
    /// Destructor
    virtual ~CASox();
    
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
    
    /// Get number of active contexts
    int getNumContexts() const { return contexts.size(); }
    
    /// Get current context index
    int getCurrentContext() const { return static_cast<int>(currentContext); }
    
    /// Get active context (alias for getCurrentContext)
    int getActiveContext() const { return getCurrentContext(); }
    
    /// Get number of contexts that have been discovered/active
    int getNumberOfActiveContexts() const { return contexts.size(); }
    
protected:
    /// Detect current context from sensor values
    int detectContext(const matrix::Matrix& x);
    
    /// Create a new context
    void createNewContext(const matrix::Matrix& x);
    
    /// Update context activations using gating network
    void updateContextActivations(const matrix::Matrix& x);
    
    /// Merge similar contexts
    void mergeContexts();
    
    /// Learn forward models for active contexts
    void learnContextModels();
    
    /// Learn controller with context-aware gradients
    void learnController();
    
    /// Calculate mixed prediction using gating
    matrix::Matrix getMixedPrediction(const matrix::Matrix& y, const matrix::Matrix& x_past);
    
    /// Update context statistics
    void updateContextStatistics();
    
    /// Adapt context prototypes
    void adaptContextPrototypes();
    
    /// Calculate distance to context
    double distanceToContext(const matrix::Matrix& x, const ContextModel& context);
    
    /// Gating function (softmax over contexts)
    std::vector<double> calculateGating(const matrix::Matrix& x);
    
protected:
    CASoxConf conf;                ///< Configuration
    
    // Mutable parameter references for Configurable
    int maxContexts;
    double contextRadius;
    double contextThreshold;
    double contextAdaptRate;
    double radiusAdaptRate;
    
    int number_sensors;            ///< Number of sensors
    int number_motors;             ///< Number of motors
    int buffersize;                ///< Buffer size
    int t;                         ///< Current timestep
    
    // Controller matrices
    matrix::Matrix C;              ///< Controller matrix
    matrix::Matrix h;              ///< Controller bias
    matrix::Matrix L;              ///< Jacobian matrix
    
    // Context management
    std::vector<ContextModel> contexts; ///< Context models
    paramval currentContext;       ///< Currently active context
    std::vector<double> contextGating; ///< Gating weights
    
    // Buffers
    lpzrobots::CircularBuffer<matrix::Matrix, 50> x_buffer;
    lpzrobots::CircularBuffer<matrix::Matrix, 50> y_buffer;
    matrix::Matrix x_smooth;       ///< Smoothed sensors
    
    // Parameters
    paramval epsC;                 ///< Controller learning rate
    paramval epsA;                 ///< Model learning rate
    paramval epsGate;              ///< Gate learning rate
    paramval creativity;           ///< Creativity parameter
    paramval damping;              ///< Damping parameter
    
    // Activation functions
    static double g(double x) { return tanh(x); }
    static double g_s(double x) { double y = tanh(x); return 1.0 - y*y; }
    static double clip(double r, double x) { return x < -r ? -r : (x > r ? r : x); }
};

#endif // __CASOX_H