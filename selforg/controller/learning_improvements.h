/***************************************************************************
 *   Learning Improvements for Homeokinetic Controllers                    *
 *                                                                         *
 *   Mathematical fixes and optimizations for better convergence          *
 ***************************************************************************/
#ifndef __LEARNING_IMPROVEMENTS_H
#define __LEARNING_IMPROVEMENTS_H

#include "matrix.h"
#include <cmath>

namespace lpzrobots {

/**
 * Improved learning functions for homeokinetic controllers
 */
class LearningImprovements {
public:
    /**
     * Adaptive learning rate based on prediction error
     * Reduces learning rate when error is high to prevent instability
     */
    static double adaptiveLearningRate(double base_rate, double error, 
                                      double min_rate = 0.001, double max_rate = 0.1) {
        // Sigmoid-based adaptation
        double factor = 1.0 / (1.0 + error * error);
        double rate = base_rate * factor;
        
        // Clamp to safe range
        return std::max(min_rate, std::min(max_rate, rate));
    }
    
    /**
     * Momentum-based updates for smoother learning
     * Helps escape local minima and reduces oscillations
     */
    class MomentumUpdater {
    public:
        MomentumUpdater(double momentum = 0.9) : momentum_factor(momentum) {}
        
        matrix::Matrix update(const matrix::Matrix& gradient, 
                             const matrix::Matrix& prev_velocity) {
            // v_t = momentum * v_{t-1} + (1 - momentum) * gradient
            return prev_velocity * momentum_factor + gradient * (1.0 - momentum_factor);
        }
        
    private:
        double momentum_factor;
    };
    
    /**
     * Gradient clipping to prevent exploding gradients
     */
    static matrix::Matrix clipGradient(const matrix::Matrix& gradient, double max_norm = 1.0) {
        double norm = sqrt(gradient.norm_sqr());
        if (norm > max_norm) {
            return gradient * (max_norm / norm);
        }
        return gradient;
    }
    
    /**
     * Natural gradient descent for controller updates
     * Takes into account the geometry of the parameter space
     */
    static matrix::Matrix naturalGradient(const matrix::Matrix& gradient,
                                         const matrix::Matrix& fisher_info,
                                         double regularization = 0.01) {
        // Add regularization to Fisher information matrix
        matrix::Matrix F_reg = fisher_info;
        for (int i = 0; i < F_reg.getM(); i++) {
            F_reg.val(i, i) += regularization;
        }
        
        // Natural gradient = F^{-1} * gradient
        return F_reg.pseudoInverse() * gradient;
    }
    
    /**
     * Stability-aware anti-Hebbian factor
     * Dynamically adjusts the strength based on system stability
     */
    static double stabilityAwareSense(double base_sense, double stability_index,
                                     double min_sense = 0.5, double max_sense = 2.0) {
        // Increase anti-Hebbian when too stable (to promote exploration)
        // Decrease when unstable (to promote convergence)
        double factor = 2.0 - stability_index;  // High when unstable, low when stable
        double sense = base_sense * factor;
        
        return std::max(min_sense, std::min(max_sense, sense));
    }
    
    /**
     * Improved weight initialization using Xavier/He initialization
     */
    static void improvedInitialization(matrix::Matrix& weights, int fan_in, int fan_out,
                                       const std::string& activation = "tanh") {
        double scale;
        if (activation == "tanh") {
            // Xavier initialization for tanh
            scale = sqrt(2.0 / (fan_in + fan_out));
        } else if (activation == "relu") {
            // He initialization for ReLU-like
            scale = sqrt(2.0 / fan_in);
        } else {
            // Default uniform
            scale = sqrt(6.0 / (fan_in + fan_out));
        }
        
        // Initialize with scaled random values
        for (int i = 0; i < weights.getM(); i++) {
            for (int j = 0; j < weights.getN(); j++) {
                weights.val(i, j) = (drand48() * 2.0 - 1.0) * scale;
            }
        }
    }
    
    /**
     * Learning rate scheduler with warm-up
     */
    class LearningRateScheduler {
    public:
        LearningRateScheduler(double initial_lr, double target_lr, 
                             int warmup_steps = 100, double decay_factor = 0.999)
            : initial_lr(initial_lr), target_lr(target_lr), 
              warmup_steps(warmup_steps), decay_factor(decay_factor), step(0) {}
        
        double getLearningRate() {
            step++;
            
            if (step <= warmup_steps) {
                // Linear warm-up
                double progress = step / (double)warmup_steps;
                return initial_lr + (target_lr - initial_lr) * progress;
            } else {
                // Exponential decay after warm-up
                int decay_steps = step - warmup_steps;
                return target_lr * pow(decay_factor, decay_steps);
            }
        }
        
    private:
        double initial_lr;
        double target_lr;
        int warmup_steps;
        double decay_factor;
        int step;
    };
    
    /**
     * Regularization techniques
     */
    struct Regularization {
        // L2 regularization (weight decay)
        static matrix::Matrix l2Regularization(const matrix::Matrix& weights, double lambda) {
            return weights * (-lambda);
        }
        
        // L1 regularization (sparsity)
        static matrix::Matrix l1Regularization(const matrix::Matrix& weights, double lambda) {
            matrix::Matrix reg(weights.getM(), weights.getN());
            for (int i = 0; i < weights.getM(); i++) {
                for (int j = 0; j < weights.getN(); j++) {
                    double w = weights.val(i, j);
                    reg.val(i, j) = w > 0 ? -lambda : (w < 0 ? lambda : 0);
                }
            }
            return reg;
        }
        
        // Elastic net (combination of L1 and L2)
        static matrix::Matrix elasticNet(const matrix::Matrix& weights, 
                                        double lambda1, double lambda2) {
            return l1Regularization(weights, lambda1) + l2Regularization(weights, lambda2);
        }
    };
    
    /**
     * Noise injection for exploration
     */
    static matrix::Matrix explorationNoise(const matrix::Matrix& shape, 
                                          double scale = 0.01,
                                          const std::string& distribution = "gaussian") {
        matrix::Matrix noise(shape.getM(), shape.getN());
        
        if (distribution == "gaussian") {
            // Box-Muller transform for Gaussian noise
            for (int i = 0; i < noise.getM(); i++) {
                for (int j = 0; j < noise.getN(); j++) {
                    double u1 = drand48();
                    double u2 = drand48();
                    double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
                    noise.val(i, j) = z * scale;
                }
            }
        } else if (distribution == "uniform") {
            for (int i = 0; i < noise.getM(); i++) {
                for (int j = 0; j < noise.getN(); j++) {
                    noise.val(i, j) = (drand48() * 2.0 - 1.0) * scale;
                }
            }
        }
        
        return noise;
    }
    
    /**
     * Batch normalization for controller outputs
     */
    class BatchNormalizer {
    public:
        BatchNormalizer(double momentum = 0.9, double epsilon = 1e-5)
            : momentum(momentum), epsilon(epsilon), initialized(false) {}
        
        matrix::Matrix normalize(const matrix::Matrix& input) {
            if (!initialized) {
                running_mean = matrix::Matrix(input.getM(), 1);
                running_var = matrix::Matrix(input.getM(), 1);
                running_var.toId();  // Initialize to 1
                initialized = true;
            }
            
            // Calculate batch statistics
            matrix::Matrix mean(input.getM(), 1);
            matrix::Matrix var(input.getM(), 1);
            
            for (int i = 0; i < input.getM(); i++) {
                mean.val(i, 0) = input.val(i, 0);
                var.val(i, 0) = 0;  // Single sample, no variance
            }
            
            // Update running statistics
            running_mean = running_mean * momentum + mean * (1.0 - momentum);
            running_var = running_var * momentum + var * (1.0 - momentum);
            
            // Normalize
            matrix::Matrix normalized(input.getM(), 1);
            for (int i = 0; i < input.getM(); i++) {
                double std = sqrt(running_var.val(i, 0) + epsilon);
                normalized.val(i, 0) = (input.val(i, 0) - running_mean.val(i, 0)) / std;
            }
            
            return normalized;
        }
        
    private:
        double momentum;
        double epsilon;
        bool initialized;
        matrix::Matrix running_mean;
        matrix::Matrix running_var;
    };
};

} // namespace lpzrobots

#endif // __LEARNING_IMPROVEMENTS_H