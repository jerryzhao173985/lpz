/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   APEX-Sox: Adaptive Predictive EXploration Sox                         *
 *   The ultimate homeokinetic controller combining all best features      *
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
#ifndef __APEXSOX_H
#define __APEXSOX_H

#include "abstractcontroller.h"
#include "controller_misc.h"
#include "../matrix/matrix.h"
#include "../utils/circular_buffer.h"
#include <vector>
#include <deque>
#include <memory>
#include <cmath>

/**
 * APEX-Sox: Adaptive Predictive EXploration Sox
 * 
 * This is the ultimate homeokinetic controller that combines:
 * 1. Adaptive horizons from AH-Sox
 * 2. Multi-scale processing from MSHC
 * 3. Context awareness from CASox
 * 4. Information maximization from IT-Sox
 * 5. Additional enhancements:
 *    - Predictive diversity bonus
 *    - Meta-learning of parameters
 *    - Stability-guided exploration
 *    - Curiosity-driven adaptation
 * 
 * Key Innovations:
 * - Unified framework combining all successful features
 * - Automatic feature selection based on environment
 * - Meta-learning adjusts algorithm parameters online
 * - Predictive diversity ensures rich behaviors
 * - Stability monitoring prevents dangerous states
 */
class APEXSox : public AbstractController {
public:
    /// Configuration structure
    struct APEXConf {
        // Core parameters
        double initFeedbackStrength;
        double learningRateC;
        double learningRateA;
        double damping;
        
        // Adaptive features
        bool useAdaptiveHorizons;
        int maxHorizon;
        double horizonAdaptRate;
        
        // Multi-scale features
        bool useMultiScale;
        std::vector<int> scaleHorizons;
        double scaleCouplingSoftness;
        
        // Context features
        bool useContexts;
        int maxContexts;
        double contextRadius;
        double contextCreationThreshold;
        
        // Information features
        bool useInfoMax;
        double infoWeight;
        double entropyBonus;
        int infoHistorySize;
        
        // Meta-learning
        bool useMetaLearning;
        double metaLearningRate;
        double parameterNoiseLevel;
        
        // Predictive diversity
        bool usePredictiveDiversity;
        double diversityWeight;
        int diversityWindow;
        
        // Stability monitoring
        bool useStabilityGuide;
        double maxAllowedError;
        double stabilityThreshold;
        
        // Curiosity
        bool useCuriosity;
        double curiosityDecay;
        double curiosityBonus;
    };
    
    /// Returns default configuration with all features enabled
    static APEXConf getDefaultConf() {
        APEXConf conf;
        
        // Core
        conf.initFeedbackStrength = 1.0;
        conf.learningRateC = 0.1;
        conf.learningRateA = 0.1;
        conf.damping = 0.00001;
        
        // Adaptive horizons
        conf.useAdaptiveHorizons = true;
        conf.maxHorizon = 10;
        conf.horizonAdaptRate = 0.01;
        
        // Multi-scale
        conf.useMultiScale = true;
        conf.scaleHorizons = {1, 5, 20};
        conf.scaleCouplingSoftness = 0.1;
        
        // Context
        conf.useContexts = true;
        conf.maxContexts = 5;
        conf.contextRadius = 0.3;
        conf.contextCreationThreshold = 0.5;
        
        // Information
        conf.useInfoMax = true;
        conf.infoWeight = 0.1;
        conf.entropyBonus = 0.05;
        conf.infoHistorySize = 100;
        
        // Meta-learning
        conf.useMetaLearning = true;
        conf.metaLearningRate = 0.001;
        conf.parameterNoiseLevel = 0.01;
        
        // Predictive diversity
        conf.usePredictiveDiversity = true;
        conf.diversityWeight = 0.05;
        conf.diversityWindow = 50;
        
        // Stability
        conf.useStabilityGuide = true;
        conf.maxAllowedError = 2.0;
        conf.stabilityThreshold = 0.1;
        
        // Curiosity
        conf.useCuriosity = true;
        conf.curiosityDecay = 0.999;
        conf.curiosityBonus = 0.1;
        
        return conf;
    }
    
    /// Constructor
    explicit APEXSox(const APEXConf& conf = getDefaultConf());
    
    /// Destructor
    virtual ~APEXSox();
    
    /// Initializes the controller
    virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr);
    
    /// Returns the number of sensors
    virtual int getSensorNumber() const override { return number_sensors; }
    
    /// Returns the number of motors
    virtual int getMotorNumber() const override { return number_motors; }
    
    /// Performs one step with learning
    virtual void step(const sensor* sensors, int sensornumber,
                     motor* motors, int motornumber);
    
    /// Performs one step without learning
    virtual void stepNoLearning(const sensor* sensors, int sensornumber,
                               motor* motors, int motornumber);
    
    /// Store controller state
    virtual bool store(FILE* f) const;
    
    /// Restore controller state
    virtual bool restore(FILE* f);
    
    /// Get current effective complexity (meta-parameter)
    double getComplexity() const { return currentComplexity; }
    
    /// Get current exploration level
    double getExplorationLevel() const { return currentExploration; }
    
    /// Get predictive diversity score
    double getPredictiveDiversity() const { return predictiveDiversity; }
    
    /// Get stability index
    double getStabilityIndex() const { return stabilityIndex; }
    
protected:
    APEXConf conf;
    unsigned short number_sensors;
    unsigned short number_motors;
    
    // Core components (unified architecture)
    struct UnifiedModel {
        matrix::Matrix C;  // Controller
        matrix::Matrix A;  // Forward model
        matrix::Matrix S;  // Extended model
        matrix::Matrix h;  // Controller bias
        matrix::Matrix b;  // Model bias
        
        // Feature-specific extensions
        int horizon;                    // Current prediction horizon
        double scale_weight;            // Weight in multi-scale
        int context_id;                 // Associated context
        double uncertainty;             // Model uncertainty
        double information_gain;        // Recent MI contribution
        double diversity_contribution;  // Predictive diversity
        
        // Performance tracking
        std::deque<double> error_history;
        double avg_error;
        double stability_score;
    };
    
    // Model ensemble
    std::vector<std::unique_ptr<UnifiedModel>> models;
    int activeModelIndex;
    
    // Buffers
    static constexpr int buffersize = 50;
    lpzrobots::CircularBuffer<matrix::Matrix, buffersize> x_buffer;
    lpzrobots::CircularBuffer<matrix::Matrix, buffersize> y_buffer;
    
    // Meta-learning state
    struct MetaState {
        double currentComplexity;      // 0=simple, 1=complex
        double currentExploration;     // 0=exploit, 1=explore
        double currentStability;       // 0=unstable, 1=stable
        double currentCuriosity;       // Decaying curiosity
        
        // Learned meta-parameters
        double optimalNoiseLevel;
        double optimalLearningRate;
        double optimalDamping;
        
        // Performance history for meta-learning
        std::deque<double> performance_history;
    };
    MetaState metaState;
    
    // Information state
    struct InfoState {
        std::deque<matrix::Matrix> sensor_history;
        std::deque<matrix::Matrix> motor_history;
        double currentMI;
        double currentEntropy;
        double adaptiveKernelWidth;
    };
    InfoState infoState;
    
    // Predictive diversity
    std::vector<matrix::Matrix> prediction_ensemble;
    double predictiveDiversity;
    
    // Stability monitoring
    double stabilityIndex;
    int instabilityCounter;
    
    // Internal state
    matrix::Matrix x_smooth;
    double currentComplexity;
    double currentExploration;
    int t;
    
    // Configurable parameters
    paramval epsC;
    paramval epsA;
    
    /// Initialize internal structures
    void constructor();
    
    /// Main learning algorithm
    virtual void learn();
    
    /// Select best model based on current state
    int selectBestModel();
    
    /// Update model ensemble
    void updateModelEnsemble();
    
    /// Meta-learning update
    void updateMetaParameters();
    
    /// Calculate predictive diversity
    double calculatePredictiveDiversity();
    
    /// Monitor stability
    void monitorStability(const matrix::Matrix& error);
    
    /// Apply curiosity bonus
    matrix::Matrix applyCuriosityBonus(const matrix::Matrix& y);
    
    /// Estimate mutual information
    double estimateMI() const;
    
    /// Adaptive parameter adjustment
    void adaptParameters();
    
    /// Create new context if needed
    void checkContextCreation(const matrix::Matrix& x);
    
    /// Blend outputs from multiple models
    matrix::Matrix blendOutputs(const std::vector<matrix::Matrix>& outputs);
    
    /// Transfer function
    static double g(double z) { return tanh(z); }
    
    /// Derivative of transfer function
    static double g_s(double z) {
        double k = tanh(z);
        return 1.0 - k * k;
    }
    
    /// Clipping function
    static double clip(double r, double x) {
        return x < -r ? -r : (x > r ? r : x);
    }
};

#endif // __APEXSOX_H