/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   APEX-Sox: Adaptive Predictive EXploration Sox Implementation          *
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

#include "apexsox.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cassert>

using namespace matrix;
using namespace std;

APEXSox::APEXSox(const APEXConf& conf)
    : AbstractController("APEXSox", "1.0")
    , conf(conf)
    , number_sensors(0)
    , number_motors(0)
    , activeModelIndex(0)
    , predictiveDiversity(0.0)
    , stabilityIndex(1.0)
    , instabilityCounter(0)
    , currentComplexity(0.5)
    , currentExploration(0.5)
    , t(0) {
    constructor();
}

APEXSox::~APEXSox() {}

void APEXSox::constructor() {
    // Add configurable parameters
    addParameterDef("epsC", &epsC, conf.learningRateC, 0, 1, 
                    "learning rate of the controller");
    addParameterDef("epsA", &epsA, conf.learningRateA, 0, 1, 
                    "learning rate of the model");
    
    // Add inspectable values
    addInspectableValue("complexity", &currentComplexity, 
                        "current system complexity");
    addInspectableValue("exploration", &currentExploration, 
                        "current exploration level");
    addInspectableValue("diversity", &predictiveDiversity, 
                        "predictive diversity score");
    addInspectableValue("stability", &stabilityIndex, 
                        "system stability index");
    
    // Initialize meta-state
    metaState.currentComplexity = 0.5;
    metaState.currentExploration = 0.5;
    metaState.currentStability = 1.0;
    metaState.currentCuriosity = 1.0;
    metaState.optimalNoiseLevel = 0.1;
    metaState.optimalLearningRate = conf.learningRateC;
    metaState.optimalDamping = conf.damping;
}

void APEXSox::init(int sensornumber, int motornumber, RandGen* randGen) {
    number_sensors = sensornumber;
    number_motors = motornumber;
    
    // Initialize buffers
    for (int k = 0; k < buffersize; ++k) {
        x_buffer.push(Matrix(number_sensors, 1));
        y_buffer.push(Matrix(number_motors, 1));
    }
    
    // Create initial model ensemble
    models.clear();
    
    // 1. Create models for different horizons (if enabled)
    if (conf.useAdaptiveHorizons) {
        for (int h = 1; h <= min(conf.maxHorizon, 5); h++) {
            auto model = make_unique<UnifiedModel>();
            model->horizon = h;
            model->scale_weight = 1.0 / min(conf.maxHorizon, 5);
            model->context_id = -1;
            model->uncertainty = 1.0;
            model->information_gain = 0.0;
            model->diversity_contribution = 0.0;
            model->avg_error = 0.0;
            model->stability_score = 1.0;
            
            // Initialize matrices
            model->C.set(number_motors, number_sensors);
            model->A.set(number_sensors, number_motors);
            model->S.set(number_sensors, number_sensors);
            model->h.set(number_motors, 1);
            model->b.set(number_sensors, 1);
            
            model->C.toId();
            model->C *= conf.initFeedbackStrength;
            model->A.toId();
            model->S.toId();
            model->S *= 0.05;
            
            models.push_back(std::move(model));
        }
    }
    
    // 2. Create models for different scales (if enabled)
    if (conf.useMultiScale) {
        for (size_t i = 0; i < conf.scaleHorizons.size(); i++) {
            auto model = make_unique<UnifiedModel>();
            model->horizon = conf.scaleHorizons[i];
            model->scale_weight = 1.0 / conf.scaleHorizons.size();
            model->context_id = -2 - i; // Negative IDs for scale models
            model->uncertainty = 1.0;
            
            // Initialize matrices with scale-specific parameters
            model->C.set(number_motors, number_sensors);
            model->A.set(number_sensors, number_motors);
            model->S.set(number_sensors, number_sensors);
            model->h.set(number_motors, 1);
            model->b.set(number_sensors, 1);
            
            model->C.toId();
            model->C *= conf.initFeedbackStrength * (1.0 + i * 0.2); // Varying strength
            model->A.toId();
            
            models.push_back(std::move(model));
        }
    }
    
    // 3. Create at least one default model
    if (models.empty()) {
        auto model = make_unique<UnifiedModel>();
        model->horizon = 1;
        model->scale_weight = 1.0;
        model->context_id = 0;
        
        model->C.set(number_motors, number_sensors);
        model->A.set(number_sensors, number_motors);
        model->S.set(number_sensors, number_sensors);
        model->h.set(number_motors, 1);
        model->b.set(number_sensors, 1);
        
        model->C.toId();
        model->C *= conf.initFeedbackStrength;
        model->A.toId();
        
        models.push_back(std::move(model));
    }
    
    // Initialize smooth sensor values
    x_smooth.set(number_sensors, 1);
    
    // Clear histories
    infoState.sensor_history.clear();
    infoState.motor_history.clear();
    infoState.currentMI = 0.0;
    infoState.currentEntropy = 0.0;
    infoState.adaptiveKernelWidth = 0.1;
    
    t = 0;
}

void APEXSox::step(const sensor* sensors, int sensornumber,
                   motor* motors, int motornumber) {
    stepNoLearning(sensors, sensornumber, motors, motornumber);
    
    // Need sufficient history for learning
    if (t < 3) return;
    
    --t; // stepNoLearning increases t
    
    // Learn if enabled
    if (epsC > 0 || epsA > 0) {
        learn();
    }
    
    ++t;
}

void APEXSox::stepNoLearning(const sensor* sensors, int sensornumber,
                             motor* motors, int motornumber) {
    assert(sensornumber == number_sensors);
    assert(motornumber == number_motors);
    
    // Get sensor values
    Matrix x(number_sensors, 1, sensors);
    x_buffer.push(x);
    
    // Smooth sensor values (simple low-pass filter)
    if (t == 0) {
        x_smooth = x;
    } else {
        x_smooth = x_smooth * 0.9 + x * 0.1;
    }
    
    // Select best model or blend multiple models
    vector<Matrix> candidate_outputs;
    vector<double> weights;
    
    if (conf.useContexts && models.size() > 1) {
        // Context-based selection
        for (size_t i = 0; i < models.size(); i++) {
            const auto& model = models[i];
            
            // Simple context matching based on recent errors
            double match_score = 1.0 / (1.0 + model->avg_error);
            if (model->error_history.size() > 5) {
                // Prefer recently successful models
                double recent_error = 0;
                for (int j = 0; j < 5; j++) {
                    recent_error += model->error_history[model->error_history.size() - 1 - j];
                }
                recent_error /= 5;
                match_score = 1.0 / (1.0 + recent_error);
            }
            
            weights.push_back(match_score * model->scale_weight);
        }
    } else {
        // Use all models with their scale weights
        for (const auto& model : models) {
            weights.push_back(model->scale_weight);
        }
    }
    
    // Normalize weights
    double weight_sum = accumulate(weights.begin(), weights.end(), 0.0);
    if (weight_sum > 0) {
        for (auto& w : weights) w /= weight_sum;
    }
    
    // Generate outputs from each model
    for (size_t i = 0; i < models.size(); i++) {
        const auto& model = models[i];
        
        // Add creativity/exploration
        Matrix x_creative = x_smooth;
        if (conf.useCuriosity && metaState.currentCuriosity > 0) {
            Matrix noise(number_sensors, 1);
            for (int j = 0; j < number_sensors; j++) {
                noise.val(j, 0) = (2.0 * rand() / RAND_MAX - 1.0) * 
                                 metaState.currentCuriosity * conf.curiosityBonus;
            }
            x_creative += noise;
        }
        
        // Controller output
        Matrix y = (model->C * x_creative + model->h).map(g);
        
        // Apply curiosity bonus if enabled
        if (conf.useCuriosity) {
            y = applyCuriosityBonus(y);
        }
        
        candidate_outputs.push_back(y);
    }
    
    // Blend outputs
    Matrix y = blendOutputs(candidate_outputs);
    
    // Store motor values
    y_buffer.push(y);
    y.convertToBuffer(motors, motornumber);
    
    // Update histories for information estimation
    if (conf.useInfoMax) {
        infoState.sensor_history.push_back(x);
        infoState.motor_history.push_back(y);
        
        if (infoState.sensor_history.size() > static_cast<size_t>(conf.infoHistorySize)) {
            infoState.sensor_history.pop_front();
            infoState.motor_history.pop_front();
        }
    }
    
    // Update predictive diversity
    if (conf.usePredictiveDiversity && t > 10) {
        predictiveDiversity = calculatePredictiveDiversity();
    }
    
    ++t;
}

void APEXSox::learn() {
    // Get delayed values for learning
    const Matrix& x = x_buffer.get(-1);
    const Matrix& y = y_buffer.get(-1);
    const Matrix& x_fut = x_buffer.get(0);
    
    // Update each model
    for (auto& model : models) {
        // Predict future sensor values
        const Matrix& x_pred = model->A * y + model->b + model->S * x;
        const Matrix& xi = x_fut - x_pred;
        
        // Update error history
        double error = xi.norm_sqr();
        model->error_history.push_back(error);
        if (model->error_history.size() > 20) {
            model->error_history.pop_front();
        }
        
        // Calculate average error
        if (!model->error_history.empty()) {
            model->avg_error = accumulate(model->error_history.begin(), 
                                        model->error_history.end(), 0.0) / 
                              model->error_history.size();
        }
        
        // Homeokinetic learning with meta-learned parameters
        const Matrix& z = model->C * x + model->h;
        const Matrix& y_g = z.map(g);
        const Matrix& g_prime = z.map(g_s);
        
        // Jacobian
        Matrix L = model->A * model->C.multrowwise(g_prime) + model->S;
        
        // Pseudo-inverse with regularization
        Matrix Lplus = L.pseudoInverse(0.01);
        
        const Matrix& v = Lplus * xi;
        const Matrix& chi = (Lplus^T) * v;
        const Matrix& mu = ((model->A^T) & g_prime) * chi;
        
        // Update with meta-learned rates
        double epsC_meta = metaState.optimalLearningRate;
        double epsA_meta = metaState.optimalLearningRate;
        
        // Anti-Hebbian learning with adaptive sense
        double sense = 1.0 + metaState.currentExploration * 0.5; // Adaptive sense based on exploration
        Matrix epsrel = (mu & (model->C * v)) * (sense * 2.0);
        model->C += ((mu * (v^T) - epsrel.multrowwise(y_g) * (x^T)) * epsC_meta).mapP(0.05, clip);
        model->h += ((mu - epsrel.multrowwise(y_g)) * epsC_meta).mapP(0.05, clip);
        
        // Model learning
        model->A += (xi * (y^T) * epsA_meta).mapP(0.1, clip);
        model->b += (xi * epsA_meta * 0.1).mapP(0.1, clip);
        model->S += (xi * (x^T) * epsA_meta * 0.1).mapP(0.1, clip);
        
        // Apply damping
        model->A += (model->A * (-metaState.optimalDamping)).mapP(0.1, clip);
        model->C += (model->C * (-metaState.optimalDamping)).mapP(0.05, clip);
    }
    
    // Meta-learning updates
    if (conf.useMetaLearning && t % 10 == 0) {
        updateMetaParameters();
    }
    
    // Update model ensemble (create/remove models)
    if (t % 50 == 0) {
        updateModelEnsemble();
    }
    
    // Monitor stability
    const Matrix& xi_global = x_fut - x_buffer.get(-1);
    monitorStability(xi_global);
    
    // Update curiosity
    if (conf.useCuriosity) {
        metaState.currentCuriosity *= conf.curiosityDecay;
    }
    
    // Information maximization update
    if (conf.useInfoMax && infoState.sensor_history.size() > 10) {
        infoState.currentMI = estimateMI();
        
        // Adjust exploration based on information gain
        if (infoState.currentMI < 0.1) { // Low information
            currentExploration = min(1.0, currentExploration + 0.01);
        } else {
            currentExploration = max(0.1, currentExploration - 0.01);
        }
    }
}

int APEXSox::selectBestModel() {
    double best_score = -1e10;
    int best_idx = 0;
    
    for (size_t i = 0; i < models.size(); i++) {
        const auto& model = models[i];
        
        // Composite score considering multiple factors
        double score = 0;
        
        // Low error is good
        score -= model->avg_error;
        
        // High stability is good
        score += model->stability_score * 0.5;
        
        // Information gain is good (if available)
        score += model->information_gain * conf.infoWeight;
        
        // Diversity contribution is good
        score += model->diversity_contribution * conf.diversityWeight;
        
        // Uncertainty penalty (prefer confident models)
        score -= model->uncertainty * 0.1;
        
        if (score > best_score) {
            best_score = score;
            best_idx = i;
        }
    }
    
    return best_idx;
}

void APEXSox::updateModelEnsemble() {
    // Remove poorly performing models
    if (models.size() > 3) {
        // Find worst performing model
        size_t worst_idx = 0;
        double worst_score = 1e10;
        
        for (size_t i = 0; i < models.size(); i++) {
            if (models[i]->avg_error > worst_score) {
                worst_score = models[i]->avg_error;
                worst_idx = i;
            }
        }
        
        // Remove if significantly worse than average
        double avg_error = 0;
        for (const auto& model : models) {
            avg_error += model->avg_error;
        }
        avg_error /= models.size();
        
        if (worst_score > avg_error * 2.0) {
            models.erase(models.begin() + worst_idx);
        }
    }
    
    // Add new model if performance is plateauing
    if (models.size() < static_cast<size_t>(conf.maxContexts)) {
        // Check if all models have similar performance
        double error_variance = 0;
        double avg_error = 0;
        for (const auto& model : models) {
            avg_error += model->avg_error;
        }
        avg_error /= models.size();
        
        for (const auto& model : models) {
            double diff = model->avg_error - avg_error;
            error_variance += diff * diff;
        }
        error_variance /= models.size();
        
        // If all models perform similarly, add diversity
        if (error_variance < 0.01) {
            auto new_model = make_unique<UnifiedModel>();
            new_model->horizon = 1 + rand() % conf.maxHorizon;
            new_model->scale_weight = 1.0 / (models.size() + 1);
            new_model->context_id = models.size();
            
            // Initialize with variation
            new_model->C.set(number_motors, number_sensors);
            new_model->A.set(number_sensors, number_motors);
            new_model->S.set(number_sensors, number_sensors);
            new_model->h.set(number_motors, 1);
            new_model->b.set(number_sensors, 1);
            
            new_model->C.toId();
            new_model->C *= conf.initFeedbackStrength * (0.5 + rand() / double(RAND_MAX));
            new_model->A.toId();
            
            models.push_back(std::move(new_model));
            
            // Renormalize weights
            for (auto& model : models) {
                model->scale_weight = 1.0 / models.size();
            }
        }
    }
}

void APEXSox::updateMetaParameters() {
    // Calculate recent performance
    double recent_performance = 0;
    int count = 0;
    
    for (const auto& model : models) {
        if (!model->error_history.empty()) {
            // Performance = inverse of error + diversity + stability
            recent_performance += 1.0 / (1.0 + model->avg_error);
            recent_performance += model->stability_score * 0.5;
            recent_performance += predictiveDiversity * 0.3;
            count++;
        }
    }
    
    if (count > 0) {
        recent_performance /= count;
        metaState.performance_history.push_back(recent_performance);
        
        if (metaState.performance_history.size() > 20) {
            metaState.performance_history.pop_front();
        }
        
        // Adjust meta-parameters based on performance trend
        if (metaState.performance_history.size() > 5) {
            // Calculate trend
            double early_avg = 0, late_avg = 0;
            size_t half = metaState.performance_history.size() / 2;
            
            for (size_t i = 0; i < half; i++) {
                early_avg += metaState.performance_history[i];
            }
            for (size_t i = half; i < metaState.performance_history.size(); i++) {
                late_avg += metaState.performance_history[i];
            }
            
            early_avg /= half;
            late_avg /= (metaState.performance_history.size() - half);
            
            double trend = late_avg - early_avg;
            
            // Adjust parameters based on trend
            if (trend < -0.01) { // Performance decreasing
                // Increase exploration
                metaState.optimalNoiseLevel *= 1.1;
                metaState.optimalLearningRate *= 0.95;
                metaState.currentCuriosity = min(1.0, metaState.currentCuriosity + 0.1);
            } else if (trend > 0.01) { // Performance increasing
                // Refine current strategy
                metaState.optimalNoiseLevel *= 0.95;
                metaState.optimalLearningRate *= 1.05;
            }
            
            // Bounds
            metaState.optimalNoiseLevel = ::clip(metaState.optimalNoiseLevel, 0.001, 0.5);
            metaState.optimalLearningRate = ::clip(metaState.optimalLearningRate, 0.01, 0.5);
        }
    }
    
    // Update complexity estimate
    currentComplexity = 0;
    for (const auto& model : models) {
        // Complexity based on model parameters and activity
        double model_complexity = 0;
        model_complexity += model->C.norm_sqr() / (number_motors * number_sensors);
        model_complexity += model->A.norm_sqr() / (number_sensors * number_motors);
        model_complexity += abs(model->horizon - 1) / double(conf.maxHorizon);
        currentComplexity += model_complexity * model->scale_weight;
    }
    currentComplexity = ::clip(currentComplexity, 0.0, 1.0);
}

double APEXSox::calculatePredictiveDiversity() {
    if (models.size() < 2) return 0.0;
    
    // Get current input
    const Matrix& x = x_buffer.get(0);
    
    // Collect predictions from all models
    prediction_ensemble.clear();
    for (const auto& model : models) {
        Matrix y_pred = (model->C * x + model->h).map(g);
        prediction_ensemble.push_back(y_pred);
    }
    
    // Calculate diversity as average pairwise distance
    double diversity = 0;
    int pairs = 0;
    
    for (size_t i = 0; i < prediction_ensemble.size(); i++) {
        for (size_t j = i + 1; j < prediction_ensemble.size(); j++) {
            Matrix diff = prediction_ensemble[i] - prediction_ensemble[j];
            diversity += sqrt(diff.norm_sqr());
            pairs++;
        }
    }
    
    if (pairs > 0) {
        diversity /= pairs;
    }
    
    return diversity;
}

void APEXSox::monitorStability(const Matrix& error) {
    double error_magnitude = sqrt(error.norm_sqr());
    
    // Update stability index (exponential moving average)
    stabilityIndex = stabilityIndex * 0.95 + (1.0 - min(1.0, error_magnitude / conf.maxAllowedError)) * 0.05;
    
    // Check for instability
    if (error_magnitude > conf.maxAllowedError) {
        instabilityCounter++;
        
        // Emergency measures if unstable
        if (instabilityCounter > 5) {
            // Reduce all learning rates temporarily
            for (auto& model : models) {
                model->C *= 0.9;
                model->A *= 0.9;
            }
            
            // Increase damping
            metaState.optimalDamping *= 2.0;
            
            // Reset curiosity
            metaState.currentCuriosity = 0.1;
            
            instabilityCounter = 0;
        }
    } else {
        instabilityCounter = max(0, instabilityCounter - 1);
        
        // Slowly restore parameters if stable
        if (stabilityIndex > 0.9) {
            metaState.optimalDamping *= 0.99;
        }
    }
}

Matrix APEXSox::applyCuriosityBonus(const Matrix& y) {
    if (metaState.currentCuriosity < 0.01) return y;
    
    Matrix y_curious = y;
    
    // Add curiosity-driven exploration
    for (int i = 0; i < y.getM(); i++) {
        // Curiosity pushes away from zero (encourages movement)
        double val = y.val(i, 0);
        double push = metaState.currentCuriosity * conf.curiosityBonus;
        
        if (abs(val) < 0.3) { // Only apply to low-activity motors
            if (val >= 0) {
                y_curious.val(i, 0) = min(1.0, val + push);
            } else {
                y_curious.val(i, 0) = max(-1.0, val - push);
            }
        }
    }
    
    return y_curious;
}

double APEXSox::estimateMI() const {
    // Simplified MI estimation
    if (infoState.sensor_history.size() < 20) return 0.0;
    
    // Use recent history
    size_t n = min(size_t(50), infoState.sensor_history.size());
    
    // Calculate average prediction error with and without motor information
    double error_with_motor = 0;
    double error_without_motor = 0;
    
    for (size_t i = 1; i < n; i++) {
        // Prediction error when using motor commands
        Matrix x_pred_with = models[activeModelIndex]->A * infoState.motor_history[i-1] + 
                            models[activeModelIndex]->b;
        Matrix error1 = infoState.sensor_history[i] - x_pred_with;
        error_with_motor += error1.norm_sqr();
        
        // Prediction error without motor (just using previous state)
        Matrix error2 = infoState.sensor_history[i] - infoState.sensor_history[i-1];
        error_without_motor += error2.norm_sqr();
    }
    
    // MI approximation: reduction in prediction error
    double mi = log(error_without_motor / (error_with_motor + 0.001));
    return max(0.0, mi);
}

Matrix APEXSox::blendOutputs(const vector<Matrix>& outputs) {
    if (outputs.size() == 1) return outputs[0];
    
    // Weighted average based on model performance
    Matrix result(number_motors, 1);
    double weight_sum = 0;
    
    for (size_t i = 0; i < outputs.size(); i++) {
        double weight = 1.0 / (1.0 + models[i]->avg_error);
        result += outputs[i] * weight;
        weight_sum += weight;
    }
    
    if (weight_sum > 0) {
        result *= (1.0 / weight_sum);
    }
    
    return result;
}

bool APEXSox::store(FILE* f) const {
    // Store configuration
    fwrite(&conf, sizeof(conf), 1, f);
    
    // Store number of models
    size_t num_models = models.size();
    fwrite(&num_models, sizeof(num_models), 1, f);
    
    // Store each model
    for (const auto& model : models) {
        model->C.store(f);
        model->A.store(f);
        model->S.store(f);
        model->h.store(f);
        model->b.store(f);
        
        fwrite(&model->horizon, sizeof(model->horizon), 1, f);
        fwrite(&model->scale_weight, sizeof(model->scale_weight), 1, f);
        fwrite(&model->uncertainty, sizeof(model->uncertainty), 1, f);
    }
    
    // Store meta-state
    fwrite(&metaState, sizeof(metaState), 1, f);
    
    Configurable::print(f, 0);
    return true;
}

bool APEXSox::restore(FILE* f) {
    // Restore configuration
    fread(&conf, sizeof(conf), 1, f);
    
    // Restore models
    size_t num_models;
    fread(&num_models, sizeof(num_models), 1, f);
    
    models.clear();
    for (size_t i = 0; i < num_models; i++) {
        auto model = make_unique<UnifiedModel>();
        
        model->C.restore(f);
        model->A.restore(f);
        model->S.restore(f);
        model->h.restore(f);
        model->b.restore(f);
        
        fread(&model->horizon, sizeof(model->horizon), 1, f);
        fread(&model->scale_weight, sizeof(model->scale_weight), 1, f);
        fread(&model->uncertainty, sizeof(model->uncertainty), 1, f);
        
        models.push_back(std::move(model));
    }
    
    // Restore meta-state
    fread(&metaState, sizeof(metaState), 1, f);
    
    Configurable::parse(f);
    t = 0;
    
    return true;
}