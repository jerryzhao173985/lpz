/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Multi-Scale Homeokinetic Controller (MSHC) Implementation             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mshc.h"
#include <algorithm>
#include <cmath>
#include <numeric>

using namespace matrix;
using namespace std;

MSHC::MSHC(const MSHCConf& conf)
    : AbstractController("MSHC", "1.0")
    , conf(conf)
    , crossScaleCoupling(conf.crossScaleCoupling)
    , scaleAdaptRate(conf.scaleAdaptRate)
    , number_sensors(0)
    , number_motors(0)
    , buffersize(200)  // Large buffer for slow scale
    , t(0) {
    
    // Validate configuration
    assert(conf.scaleHorizons.size() == conf.scaleWeights.size());
    assert(!conf.scaleHorizons.empty());
    
    // Add configurable parameters
    addParameterDef("epsC", &epsC, conf.learningRateC, 0, 5, "learning rate of the controller");
    addParameterDef("epsA", &epsA, conf.learningRateA, 0, 5, "learning rate of the models");
    addParameterDef("creativity", &creativity, conf.creativity, 0, 1, "creativity term");
    addParameterDef("damping", &damping, conf.damping, 0, 0.01, "forgetting term");
    addParameter("crossCoupling", &crossScaleCoupling, 0, 1, 
                   "cross-scale coupling strength");
    addParameter("scaleAdaptRate", &scaleAdaptRate, 0, 1,
                   "scale weight adaptation rate");
    
    // Note: Scale-specific parameters will be added after scales are initialized in init()
    
    // Add inspectable matrices
    addInspectableMatrix("C_combined", &C_combined, false, "combined controller matrix");
    addInspectableMatrix("h_combined", &h_combined, false, "combined controller bias");
}

MSHC::~MSHC() {}

void MSHC::init(int sensornumber, int motornumber, RandGen* randGen) {
    assert(sensornumber > 0 && motornumber > 0);
    
    number_sensors = sensornumber;
    number_motors = motornumber;
    
    // Determine buffer size based on slowest scale
    int maxHorizon = *max_element(conf.scaleHorizons.begin(), conf.scaleHorizons.end());
    buffersize = max(200, maxHorizon * 2);
    
    // Initialize time scales
    scales.clear();
    for (size_t i = 0; i < conf.scaleHorizons.size(); i++) {
        TimeScale scale;
        scale.horizon = conf.scaleHorizons[i];
        scale.weight = conf.scaleWeights[i];
        
        // Fast scales learn faster, slow scales learn slower
        if (i == 0) {  // Fastest scale
            scale.learningRateFactor = conf.fastScaleFactor;
        } else if (i == conf.scaleHorizons.size() - 1) {  // Slowest scale
            scale.learningRateFactor = conf.slowScaleFactor;
        } else {  // Medium scales
            scale.learningRateFactor = 1.0;
        }
        
        // Initialize matrices
        scale.A = Matrix(number_sensors, number_motors);
        scale.A.toId();
        scale.A *= 0.1;
        
        scale.S = Matrix(number_sensors, number_sensors);
        scale.S.toZero();
        
        scale.b = Matrix(number_sensors, 1);
        scale.b.toZero();
        
        scale.C = Matrix(number_motors, number_sensors);
        scale.C.toId();
        scale.C *= conf.initFeedbackStrength * scale.weight;
        
        scale.h = Matrix(number_motors, 1);
        scale.h.toZero();
        
        scale.activity = 0.5;  // Start with neutral activity
        
        scales.push_back(scale);
    }
    
    // Initialize combined matrices
    C_combined = Matrix(number_motors, number_sensors);
    h_combined = Matrix(number_motors, 1);
    C_combined.toZero();
    h_combined.toZero();
    
    // Initialize buffers with zeros
    for (int i = 0; i < buffersize; i++) {
        x_buffer.push(Matrix(number_sensors, 1));
        y_buffer.push(Matrix(number_motors, 1));
    }
    
    x_smooth = Matrix(number_sensors, 1);
    x_smooth.toZero();
    
    // Normalize initial weights
    double totalWeight = accumulate(conf.scaleWeights.begin(), conf.scaleWeights.end(), 0.0);
    if (totalWeight > 0) {
        for (TimeScale& scale : scales) {
            scale.weight /= totalWeight;
        }
    }
    
    t = 0;
}

void MSHC::stepNoLearning(const sensor* x_, int number_sensors_robot,
                          motor* y_, int number_motors_robot) {
    assert(number_sensors_robot <= number_sensors && number_motors_robot <= number_motors);
    
    // Store sensor values
    Matrix x(number_sensors, 1);
    x.set(number_sensors_robot, 1, x_);
    
    // Smooth sensor values
    x_smooth += (x - x_smooth) * 0.1;
    
    // Store in buffer
    x_buffer.push(x_smooth);
    
    // Combine outputs from all scales
    Matrix y;
    
    // Handle creativity properly
    if (creativity > 0 && !scales.empty()) {
        // Use slowest scale for creativity
        const TimeScale& slowScale = scales.back();
        if (t >= slowScale.horizon) {
            // First compute output without creativity to use for prediction
            Matrix y_temp = combineScaleOutputs(x_smooth);
            // Predict next state
            Matrix x_pred = slowScale.A * y_temp + slowScale.b;
            if (conf.useExtendedModel) {
                x_pred += slowScale.S * x_smooth;
            }
            // Apply creativity to input
            Matrix x_creative = x_smooth + (x_pred - x_smooth) * creativity;
            y = combineScaleOutputs(x_creative);
        } else {
            y = combineScaleOutputs(x_smooth);
        }
    } else {
        y = combineScaleOutputs(x_smooth);
    }
    
    // Store in buffer
    y_buffer.push(y);
    
    // Convert to motor array
    y.convertToBuffer(y_, number_motors_robot);
    
    t++;
}

void MSHC::step(const sensor* x_, int number_sensors_robot,
                motor* y_, int number_motors_robot) {
    // First do the step without learning
    stepNoLearning(x_, number_sensors_robot, y_, number_motors_robot);
    
    // Update scale activities
    updateScaleActivities();
    
    // Update scale weights if dynamic adaptation is enabled
    if (conf.dynamicScaleWeights) {
        updateScaleWeights();
    }
    
    // Learn forward models
    if (epsA > 0) {
        learnModels();
    }
    
    // Learn controllers
    if (epsC > 0) {
        learnControllers();
    }
    
    // Apply cross-scale coupling
    if (crossScaleCoupling > 0) {
        applyCrossScaleCoupling();
    }
    
    // Update combined controller matrices
    C_combined.toZero();
    h_combined.toZero();
    for (const TimeScale& scale : scales) {
        C_combined += scale.C * scale.weight;
        h_combined += scale.h * scale.weight;
    }
}

Matrix MSHC::calculateScaleTLE(const TimeScale& scale) const {
    if (t < scale.horizon) return Matrix(number_sensors, 1);
    
    // Get past values at scale's horizon
    const Matrix& y_past = getBufferedMotor(-scale.horizon);
    const Matrix& x_past = getBufferedSensor(-scale.horizon);
    const Matrix& x_now = getBufferedSensor(-1);
    
    // Predict current sensors using scale's model
    Matrix x_pred = scale.A * y_past + scale.b;
    if (conf.useExtendedModel) {
        x_pred += scale.S * x_past;
    }
    
    // Time-loop error for this scale
    return x_now - x_pred;
}

void MSHC::updateScaleActivities() {
    // Enhanced scale activity calculation with temporal coherence
    vector<double> scale_performances;
    
    for (TimeScale& scale : scales) {
        if (t < scale.horizon + 1) {
            scale_performances.push_back(0.5);  // Default for new scales
            continue;
        }
        
        // Calculate current prediction error
        Matrix tle = calculateScaleTLE(scale);
        double error = tle.norm_sqr();
        
        // Update error history
        scale.errors.push_back(error);
        if (scale.errors.size() > 30) {  // Keep more history
            scale.errors.pop_front();
        }
        
        // Enhanced activity calculation
        if (!scale.errors.empty()) {
            // Basic average error
            double avgError = accumulate(scale.errors.begin(), scale.errors.end(), 0.0) 
                            / scale.errors.size();
            
            // Calculate error variance (consistency)
            double variance = 0;
            for (double e : scale.errors) {
                variance += (e - avgError) * (e - avgError);
            }
            variance /= scale.errors.size();
            
            // Calculate temporal coherence (how well does this scale capture patterns)
            double coherence = 1.0;
            if (scale.errors.size() > 10) {
                // Auto-correlation at lag 1
                double autocorr = 0;
                for (size_t i = 1; i < scale.errors.size(); i++) {
                    autocorr += (scale.errors[i] - avgError) * (scale.errors[i-1] - avgError);
                }
                autocorr /= (scale.errors.size() - 1);
                coherence = fabs(autocorr) / (variance + 0.001);
            }
            
            // Performance metric combines accuracy, consistency, and coherence
            double accuracy = 1.0 / (1.0 + avgError);
            double consistency = 1.0 / (1.0 + sqrt(variance));
            double performance = accuracy * 0.5 + consistency * 0.3 + coherence * 0.2;
            
            // Update activity with momentum
            scale.activity = 0.9 * scale.activity + 0.1 * performance;
            scale_performances.push_back(performance);
        }
    }
    
    // Adaptive weight adjustment based on relative performance
    if (conf.dynamicScaleWeights && !scale_performances.empty()) {
        // Calculate softmax weights based on performance
        double max_perf = *max_element(scale_performances.begin(), scale_performances.end());
        vector<double> exp_perfs;
        double sum_exp = 0;
        
        for (double perf : scale_performances) {
            double exp_perf = exp(2.0 * (perf - max_perf));  // Temperature = 0.5
            exp_perfs.push_back(exp_perf);
            sum_exp += exp_perf;
        }
        
        // Update weights with momentum
        for (size_t i = 0; i < scales.size() && i < exp_perfs.size(); i++) {
            double target_weight = exp_perfs[i] / sum_exp;
            scales[i].weight = 0.95 * scales[i].weight + 0.05 * target_weight;
        }
        
        // Ensure weights sum to 1
        double sum_weights = 0;
        for (const TimeScale& scale : scales) {
            sum_weights += scale.weight;
        }
        if (sum_weights > 0) {
            for (TimeScale& scale : scales) {
                scale.weight /= sum_weights;
            }
        }
    }
}

void MSHC::updateScaleWeights() {
    // Calculate new weights based on activities
    vector<double> newWeights(scales.size());
    double totalActivity = 0;
    
    for (size_t i = 0; i < scales.size(); i++) {
        // Weight based on activity and a prior
        newWeights[i] = scales[i].activity * conf.scaleWeights[i];
        totalActivity += newWeights[i];
    }
    
    // Normalize and update with smoothing
    if (totalActivity > 0) {
        for (size_t i = 0; i < scales.size(); i++) {
            double targetWeight = newWeights[i] / totalActivity;
            scales[i].weight += (targetWeight - scales[i].weight) * scaleAdaptRate;
        }
    }
    
    // Ensure weights sum to 1
    double sumWeights = 0;
    for (const TimeScale& scale : scales) {
        sumWeights += scale.weight;
    }
    if (sumWeights > 0) {
        for (TimeScale& scale : scales) {
            scale.weight /= sumWeights;
        }
    }
}

void MSHC::learnModels() {
    for (TimeScale& scale : scales) {
        if (t < scale.horizon + 1) continue;
        
        // Get time-loop error
        Matrix xi = calculateScaleTLE(scale);
        
        // Get past values
        const Matrix& y_past = getBufferedMotor(-scale.horizon);
        const Matrix& x_past = getBufferedSensor(-scale.horizon);
        
        // Scale-specific learning rate
        double scaleEpsA = epsA * scale.learningRateFactor;
        
        // Update forward model
        scale.A += (xi * (y_past ^ T) * scaleEpsA).mapP(0.1, clip);
        scale.b += (xi * scaleEpsA * 0.1).mapP(0.1, clip);
        
        if (conf.useExtendedModel) {
            scale.S += (xi * (x_past ^ T) * scaleEpsA).mapP(0.1, clip);
        }
        
        // Apply damping
        if (damping > 0) {
            scale.A *= (1.0 - damping);
            scale.b *= (1.0 - damping);
            if (conf.useExtendedModel) {
                scale.S *= (1.0 - damping);
            }
        }
    }
}

void MSHC::learnControllers() {
    for (TimeScale& scale : scales) {
        if (t < scale.horizon + 1) continue;
        
        // Get time-loop error
        Matrix xi = calculateScaleTLE(scale);
        
        // Get past values
        const Matrix& x_past = getBufferedSensor(-scale.horizon);
        const Matrix& y_past = getBufferedMotor(-scale.horizon);
        
        // Calculate Jacobian for this scale
        Matrix z = scale.C * x_past + scale.h;
        Matrix g_prime = z.map(g_s);
        Matrix L = scale.A * scale.C.multrowwise(g_prime);
        if (conf.useExtendedModel) {
            L += scale.S;
        }
        
        // Homeokinetic learning
        Matrix Lplus = L.pseudoInverse();
        Matrix v = Lplus * xi;
        Matrix chi = (Lplus ^ T) * v;
        Matrix mu = ((scale.A ^ T) & g_prime) * chi;
        
        // Anti-Hebbian term with scale-specific modulation
        double scaleSense = 1.0 + (scale.horizon - 1) * 0.1;  // Larger horizons -> more exploration
        Matrix epsrel = (mu & (scale.C * v)) * (scaleSense * 2.0);
        
        // Scale-specific learning rate
        double scaleEpsC = epsC * scale.learningRateFactor;
        
        // Update controller
        scale.C += ((mu * (v ^ T) - epsrel.multrowwise(y_past) * (x_past ^ T)) * scaleEpsC).mapP(0.05, clip);
        scale.h += ((mu - epsrel.multrowwise(y_past)) * scaleEpsC * 0.1).mapP(0.05, clip);
    }
}

Matrix MSHC::combineScaleOutputs(const Matrix& x) const {
    Matrix y(number_motors, 1);
    y.toZero();
    
    // Weighted combination of scale outputs
    for (const TimeScale& scale : scales) {
        Matrix y_scale = (scale.C * x + scale.h).map(g);
        y += y_scale * scale.weight;
    }
    
    return y;
}

void MSHC::applyCrossScaleCoupling() {
    // Cross-scale coupling: Fast scales influence slow scales and vice versa
    
    // Fast-to-slow coupling: Fast scale errors modulate slow scale learning
    if (scales.size() >= 2) {
        for (size_t i = 0; i < scales.size() - 1; i++) {
            TimeScale& fastScale = scales[i];
            TimeScale& slowScale = scales[i + 1];
            
            // If fast scale has high activity (low error), reduce slow scale adaptation
            double modulation = 1.0 - fastScale.activity * crossScaleCoupling;
            modulation = max(0.1, modulation);  // Keep some minimum learning
            
            // Temporarily modulate slow scale controller
            slowScale.C *= modulation;
        }
    }
    
    // Slow-to-fast coupling: Slow scale predictions influence fast scale targets
    if (scales.size() >= 2) {
        for (size_t i = 1; i < scales.size(); i++) {
            TimeScale& slowScale = scales[i];
            TimeScale& fastScale = scales[i - 1];
            
            if (t >= slowScale.horizon) {
                // Use slow scale prediction to bias fast scale
                Matrix slowPred = slowScale.A * getBufferedMotor(-slowScale.horizon) + slowScale.b;
                
                // Add a small bias towards slow scale prediction
                fastScale.b += (slowPred - fastScale.b) * crossScaleCoupling * 0.01;
            }
        }
    }
}

Matrix MSHC::getBufferedSensor(int offset) const {
    if (offset > 0 || -offset >= buffersize) {
        return Matrix(number_sensors, 1);
    }
    return x_buffer.get(offset);
}

Matrix MSHC::getBufferedMotor(int offset) const {
    if (offset > 0 || -offset >= buffersize) {
        return Matrix(number_motors, 1);
    }
    return y_buffer.get(offset);
}

double MSHC::getScaleActivity(int scaleIndex) const {
    if (scaleIndex >= 0 && scaleIndex < (int)scales.size()) {
        return scales[scaleIndex].activity;
    }
    return -1;
}

double MSHC::getScaleWeight(int scaleIndex) const {
    if (scaleIndex >= 0 && scaleIndex < (int)scales.size()) {
        return scales[scaleIndex].weight;
    }
    return -1;
}

std::vector<double> MSHC::getScaleWeights() const {
    std::vector<double> weights;
    weights.reserve(scales.size());
    for (const auto& scale : scales) {
        weights.push_back(scale.weight);
    }
    return weights;
}

bool MSHC::store(FILE* f) const {
    // Store configuration
    fwrite(&conf, sizeof(conf), 1, f);
    
    // Store number of scales
    int numScales = scales.size();
    fwrite(&numScales, sizeof(int), 1, f);
    
    // Store each scale
    for (const TimeScale& scale : scales) {
        fwrite(&scale.horizon, sizeof(int), 1, f);
        fwrite(&scale.weight, sizeof(double), 1, f);
        fwrite(&scale.learningRateFactor, sizeof(double), 1, f);
        fwrite(&scale.activity, sizeof(double), 1, f);
        
        scale.A.store(f);
        scale.S.store(f);
        scale.b.store(f);
        scale.C.store(f);
        scale.h.store(f);
    }
    
    // Store combined matrices
    C_combined.store(f);
    h_combined.store(f);
    
    // Store state
    fwrite(&t, sizeof(int), 1, f);
    
    Configurable::print(f, 0);
    return true;
}

bool MSHC::restore(FILE* f) {
    // Restore configuration
    fread(&conf, sizeof(conf), 1, f);
    
    // Restore scales
    int numScales;
    fread(&numScales, sizeof(int), 1, f);
    
    scales.clear();
    for (int i = 0; i < numScales; i++) {
        TimeScale scale;
        fread(&scale.horizon, sizeof(int), 1, f);
        fread(&scale.weight, sizeof(double), 1, f);
        fread(&scale.learningRateFactor, sizeof(double), 1, f);
        fread(&scale.activity, sizeof(double), 1, f);
        
        scale.A.restore(f);
        scale.S.restore(f);
        scale.b.restore(f);
        scale.C.restore(f);
        scale.h.restore(f);
        
        scales.push_back(scale);
    }
    
    // Restore combined matrices
    C_combined.restore(f);
    h_combined.restore(f);
    
    // Restore state
    fread(&t, sizeof(int), 1, f);
    
    Configurable::parse(f);
    
    // Reset buffers
    x_buffer.clear();
    y_buffer.clear();
    for (int i = 0; i < buffersize; i++) {
        x_buffer.push(Matrix(number_sensors, 1));
        y_buffer.push(Matrix(number_motors, 1));
    }
    
    return true;
}