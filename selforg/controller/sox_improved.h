/***************************************************************************
 *   Sox with Improved Learning                                            *
 *                                                                         *
 *   Implements mathematical improvements for better convergence           *
 ***************************************************************************/
#ifndef __SOX_IMPROVED_H
#define __SOX_IMPROVED_H

#include "sox.h"
#include "learning_improvements.h"
#include "circular_buffer.h"

/**
 * Improved Sox controller with better learning dynamics
 * 
 * Key improvements:
 * - Adaptive learning rates based on prediction error
 * - Momentum-based updates for smoother learning
 * - Gradient clipping to prevent instability
 * - Stability-aware anti-Hebbian regularization
 * - Warm-up phase for learning rates
 * - Better weight initialization
 */
class SoxImproved : public Sox {
public:
    struct ImprovedConf : public SoxConf {
        // Learning improvements
        bool useAdaptiveLearning = true;
        bool useMomentum = true;
        double momentumFactor = 0.9;
        bool useGradientClipping = true;
        double maxGradientNorm = 1.0;
        bool useWarmup = true;
        int warmupSteps = 100;
        bool useRegularization = true;
        double l2Lambda = 0.0001;
        
        // Stability improvements
        bool useStabilityAwareSense = true;
        double minSense = 0.5;
        double maxSense = 2.0;
        
        // Exploration
        bool useExplorationNoise = true;
        double noiseScale = 0.01;
        double noiseDecay = 0.999;
    };
    
    static ImprovedConf getDefaultConf() {
        ImprovedConf conf;
        conf.initFeedbackStrength = 1.0;
        conf.useExtendedModel = false;
        conf.useTeaching = false;
        conf.steps4Averaging = 1;
        conf.steps4Delay = 1;
        return conf;
    }
    
    explicit SoxImproved(const ImprovedConf& conf = getDefaultConf())
        : Sox(conf.initFeedbackStrength, conf.useExtendedModel)
        , improvedConf(conf)
        , learningStep(0)
        , currentNoiseScale(conf.noiseScale) {
        // Set default learning rates
        setParam("epsC", 0.05);
        setParam("epsA", 0.05);
        setParam("sense", 1.0);
    }
    
    virtual ~SoxImproved() {}
    
    virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override {
        Sox::init(sensornumber, motornumber, randGen);
        
        // Better initialization
        if (improvedConf.useWarmup) {
            lpzrobots::LearningImprovements::improvedInitialization(C, sensornumber, motornumber, "tanh");
            lpzrobots::LearningImprovements::improvedInitialization(A, motornumber, sensornumber, "linear");
        }
        
        // Initialize momentum buffers
        if (improvedConf.useMomentum) {
            C_velocity.set(number_motors, number_sensors);
            A_velocity.set(number_sensors, number_motors);
            h_velocity.set(number_motors, 1);
            b_velocity.set(number_sensors, 1);
            
            momentumUpdater = std::make_unique<lpzrobots::LearningImprovements::MomentumUpdater>(
                improvedConf.momentumFactor);
        }
        
        // Initialize schedulers
        if (improvedConf.useWarmup) {
            C_scheduler = std::make_unique<lpzrobots::LearningImprovements::LearningRateScheduler>(
                0.001, getParam("epsC"), improvedConf.warmupSteps);
            A_scheduler = std::make_unique<lpzrobots::LearningImprovements::LearningRateScheduler>(
                0.001, getParam("epsA"), improvedConf.warmupSteps);
        }
        
        // Initialize stability tracking
        // Initialize stability buffer
    }
    
protected:
    virtual void learn() override {
        learningStep++;
        
        // Get data from buffers
        const matrix::Matrix& x_tm1 = x_buffer.get(0);
        const matrix::Matrix& y_tm1 = y_buffer.get(0);
        const matrix::Matrix& x_smooth_tm1 = x_smooth;
        
        // Calculate prediction error
        const matrix::Matrix& xi = x_tm1 - (A * y_tm1 + b);
        double prediction_error = sqrt(xi.norm_sqr());
        
        // Update stability
        updateStability(prediction_error);
        
        // Get learning rates
        double epsC = getParam("epsC");
        double epsA = getParam("epsA");
        
        if (improvedConf.useWarmup && C_scheduler && A_scheduler) {
            epsC = C_scheduler->getLearningRate();
            epsA = A_scheduler->getLearningRate();
        }
        
        if (improvedConf.useAdaptiveLearning) {
            epsC = lpzrobots::LearningImprovements::adaptiveLearningRate(epsC, prediction_error);
            epsA = lpzrobots::LearningImprovements::adaptiveLearningRate(epsA, prediction_error);
        }
        
        // Calculate gradients using standard Sox learning
        matrix::Matrix g_prime = matrix::Matrix::map(g_s, y_tm1);
        matrix::Matrix L;
        if (useExtendedModel) {
            L = A * (S & g_prime).asDiagonal() * C;
        } else {
            // Create diagonal matrix manually
            matrix::Matrix g_diag(g_prime.getM(), g_prime.getM());
            for (int i = 0; i < g_prime.getM(); i++) {
                g_diag.val(i, i) = g_prime.val(i, 0);
            }
            L = A * g_diag * C;
        }
        
        const matrix::Matrix& v = (L^T) * xi;
        const matrix::Matrix& mu = ((C^T) * (g_prime & v));
        
        // Calculate sense (with stability awareness)
        double effective_sense = conf.sense;
        if (improvedConf.useStabilityAwareSense) {
            effective_sense = lpzrobots::LearningImprovements::stabilityAwareSense(
                sense, currentStability, improvedConf.minSense, improvedConf.maxSense);
        }
        
        // Anti-Hebbian term with correct factor of 2.0
        const matrix::Matrix& epsrel = (mu & (C * v)) * (effective_sense * 2.0);
        
        // Calculate gradients
        matrix::Matrix dC = (mu * (v^T) - (epsrel & y_tm1) * (x_smooth_tm1^T)) * epsC;
        matrix::Matrix dA = xi * (y_tm1^T) * epsA;
        matrix::Matrix dh = v * epsC * conf.factorh;
        matrix::Matrix db = xi * epsA * conf.factorb;
        
        // Apply gradient clipping
        if (improvedConf.useGradientClipping) {
            dC = lpzrobots::LearningImprovements::clipGradient(dC, improvedConf.maxGradientNorm);
            dA = lpzrobots::LearningImprovements::clipGradient(dA, improvedConf.maxGradientNorm);
            dh = lpzrobots::LearningImprovements::clipGradient(dh, improvedConf.maxGradientNorm * 0.1);
            db = lpzrobots::LearningImprovements::clipGradient(db, improvedConf.maxGradientNorm * 0.1);
        }
        
        // Apply momentum if enabled
        if (improvedConf.useMomentum && momentumUpdater) {
            C_velocity = momentumUpdater->update(dC, C_velocity);
            A_velocity = momentumUpdater->update(dA, A_velocity);
            h_velocity = momentumUpdater->update(dh, h_velocity);
            b_velocity = momentumUpdater->update(db, b_velocity);
            
            // Use velocity for updates
            dC = C_velocity;
            dA = A_velocity;
            dh = h_velocity;
            db = b_velocity;
        }
        
        // Apply regularization
        if (improvedConf.useRegularization) {
            dC += lpzrobots::LearningImprovements::Regularization::l2Regularization(C, improvedConf.l2Lambda);
            dA += lpzrobots::LearningImprovements::Regularization::l2Regularization(A, improvedConf.l2Lambda);
        }
        
        // Update parameters
        C += dC;
        A += dA;
        h += dh;
        b += db;
        
        // Apply exploration noise
        if (improvedConf.useExplorationNoise && currentNoiseScale > 0.001) {
            C += lpzrobots::LearningImprovements::explorationNoise(C, currentNoiseScale, "gaussian");
            currentNoiseScale *= improvedConf.noiseDecay;
        }
        
        // Normalize if extended model
        if (useExtendedModel) {
            squashS();
        }
    }
    
    void updateStability(double prediction_error) {
        matrix::Matrix error_mat(1, 1);
        error_mat.val(0, 0) = prediction_error;
        stability_buffer.push(error_mat);
        
        if (stability_buffer.size() > 10) {
            // Calculate variance of recent errors
            double mean = 0;
            for (int i = -10; i < 0; i++) {
                mean += stability_buffer.get(i).val(0, 0);
            }
            mean /= 10;
            
            double variance = 0;
            for (int i = -10; i < 0; i++) {
                double diff = stability_buffer.get(i).val(0, 0) - mean;
                variance += diff * diff;
            }
            variance /= 10;
            
            // Low variance = high stability
            currentStability = 1.0 / (1.0 + variance);
        }
    }
    
    // Getters for monitoring
    double getCurrentStability() const { return currentStability; }
    double getCurrentNoiseScale() const { return currentNoiseScale; }
    int getLearningStep() const { return learningStep; }
    
protected:
    ImprovedConf improvedConf;
    
    // Momentum buffers
    matrix::Matrix C_velocity;
    matrix::Matrix A_velocity;
    matrix::Matrix h_velocity;
    matrix::Matrix b_velocity;
    std::unique_ptr<lpzrobots::LearningImprovements::MomentumUpdater> momentumUpdater;
    
    // Learning rate schedulers
    std::unique_ptr<lpzrobots::LearningImprovements::LearningRateScheduler> C_scheduler;
    std::unique_ptr<lpzrobots::LearningImprovements::LearningRateScheduler> A_scheduler;
    
    // Stability tracking
    lpzrobots::CircularBuffer<matrix::Matrix, 50> stability_buffer;
    double currentStability = 1.0;
    
    // Learning state
    int learningStep;
    double currentNoiseScale;
};

#endif // __SOX_IMPROVED_H