/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Adaptive Horizon Sox (AH-Sox) Implementation                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ahsox.h"
#include <algorithm>
#include <cmath>

using namespace matrix;
using namespace std;

AHSox::AHSox(const AHSoxConf& conf)
    : AbstractController("AHSox", "1.0")
    , conf(conf)
    , minHorizon(conf.minHorizon)
    , maxHorizon(conf.maxHorizon)
    , horizonAdaptRate(conf.horizonAdaptRate)
    , uncertaintyThreshold(conf.uncertaintyThreshold)
    , number_sensors(0)
    , number_motors(0)
    , buffersize(50)
    , t(0)
    , currentHorizon(conf.minHorizon) {
    
    // Add configurable parameters
    addParameterDef("epsC", &epsC, conf.learningRateC, 0, 5, "learning rate of the controller");
    addParameterDef("epsA", &epsA, conf.learningRateA, 0, 5, "learning rate of the models");
    addParameterDef("creativity", &creativity, conf.creativity, 0, 1, "creativity term");
    addParameterDef("damping", &damping, conf.damping, 0, 0.01, "forgetting term for models");
    addParameterDef("sense", &sense, 1, 0.2, 5, "sensibility");
    
    addParameter("minHorizon", &minHorizon, 1, 10, "minimum prediction horizon");
    addParameter("maxHorizon", &maxHorizon, 1, 20, "maximum prediction horizon");
    addParameter("horizonAdaptRate", &horizonAdaptRate, 0, 1, "horizon adaptation rate");
    addParameter("uncertaintyThreshold", &uncertaintyThreshold, 0, 1, "uncertainty threshold");
    
    // Inspectable values
    addInspectableValue("currentHorizon", &currentHorizon, "currently selected horizon");
    addInspectableMatrix("C", &C, false, "controller matrix");
    addInspectableMatrix("h", &h, false, "controller bias");
    addInspectableMatrix("L", &L, false, "Jacobian matrix");
}

AHSox::~AHSox() {}

void AHSox::init(int sensornumber, int motornumber, RandGen* randGen) {
    assert(sensornumber > 0 && motornumber > 0);
    
    number_sensors = sensornumber;
    number_motors = motornumber;
    
    // Initialize controller matrices
    C = Matrix(number_motors, number_sensors);
    h = Matrix(number_motors, 1);
    L = Matrix(number_sensors, number_motors);
    
    // Initialize controller with small random values
    C.toId();
    C *= conf.initFeedbackStrength;
    h.toZero();
    
    // Initialize forward models for each horizon
    models.clear();
    for (int horizon = minHorizon; horizon <= maxHorizon; horizon++) {
        ForwardModel model;
        model.A = Matrix(number_sensors, number_motors);
        model.S = Matrix(number_sensors, number_sensors);
        model.b = Matrix(number_sensors, 1);
        
        // Initialize with small random values
        model.A.toId();
        model.A *= 0.1;
        model.S.toZero();
        model.b.toZero();
        
        model.uncertainty = 1.0;  // Start with high uncertainty
        model.weight = 1.0 / (maxHorizon - minHorizon + 1);  // Equal initial weights
        
        models.push_back(model);
    }
    
    // Initialize buffers
    // CircularBuffer automatically handles initialization
    buffersize = 50;  // Standard buffer size
    
    x_smooth = Matrix(number_sensors, 1);
    x_smooth.toZero();
    
    t = 0;
}

void AHSox::stepNoLearning(const sensor* x_, int number_sensors_robot,
                           motor* y_, int number_motors_robot) {
    assert(number_sensors_robot <= number_sensors && number_motors_robot <= number_motors);
    
    // Store sensor values
    Matrix x(number_sensors, 1);
    x.set(number_sensors_robot, 1, x_);
    
    // Simple smoothing
    x_smooth += (x - x_smooth) * 0.1;
    
    // Store in buffer
    x_buffer.push(x_smooth);
    
    // Calculate motor values using controller
    Matrix y;
    
    // Add creativity if enabled
    if (creativity > 0) {
        // Use prediction from current best model as creativity source
        int bestIdx = static_cast<int>(currentHorizon) - minHorizon;
        if (bestIdx >= 0 && bestIdx < static_cast<int>(models.size())) {
            const ForwardModel& model = models[bestIdx];
            // First compute what the output would be without creativity
            Matrix y_temp = (C * x_smooth + h).map(g);
            // Predict next state
            Matrix x_pred = model.A * y_temp + model.b;
            // Apply creativity to input, not output
            Matrix x_creative = x_smooth + (x_pred - x_smooth) * creativity;
            y = (C * x_creative + h).map(g);
        } else {
            y = (C * x_smooth + h).map(g);
        }
    } else {
        y = (C * x_smooth + h).map(g);
    }
    
    // Store in buffer
    y_buffer.push(y);
    
    // Convert to motor array
    y.convertToBuffer(y_, number_motors_robot);
    
    t++;
}

void AHSox::step(const sensor* x_, int number_sensors_robot,
                motor* y_, int number_motors_robot) {
    // First do the step without learning
    stepNoLearning(x_, number_sensors_robot, y_, number_motors_robot);
    
    // Only start learning after we have enough data
    if (t < maxHorizon + 2) return;
    
    // Update uncertainties based on recent predictions
    updateUncertainties();
    
    // Update horizon selection/weights
    updateHorizonWeights();
    
    // Select best horizon or use weighted combination
    if (!conf.useWeightedCombination) {
        currentHorizon = static_cast<double>(selectBestHorizon());
    }
    
    // Learn forward models
    if (epsA > 0) {
        learnModels();
    }
    
    // Learn controller
    if (epsC > 0) {
        learnController();
    }
}

Matrix AHSox::calculateTLE(int horizon) const {
    if (t < horizon) return Matrix(number_sensors, 1);
    
    // Get past motor command
    const Matrix& y_past = y_buffer.get(-horizon);
    const Matrix& x_past = x_buffer.get(-horizon);
    const Matrix& x_now = x_buffer.get(-1);
    
    // Get the appropriate model
    int modelIdx = horizon - minHorizon;
    if (modelIdx < 0 || modelIdx >= static_cast<int>(models.size())) {
        return Matrix(number_sensors, 1);
    }
    
    const ForwardModel& model = models[modelIdx];
    
    // Predict current sensors from past motors
    Matrix x_pred = model.A * y_past + model.b;
    if (conf.useExtendedModel) {
        x_pred += model.S * x_past;
    }
    
    // Time-loop error
    return x_now - x_pred;
}

void AHSox::updateUncertainties() {
    for (int horizon = minHorizon; horizon <= maxHorizon; horizon++) {
        int idx = horizon - minHorizon;
        if (idx < 0 || idx >= static_cast<int>(models.size())) continue;
        
        ForwardModel& model = models[idx];
        
        // Calculate current prediction error
        Matrix tle = calculateTLE(horizon);
        double error = tle.norm_sqr();
        
        // Update error history
        model.errorHistory.push_back(error);
        if (model.errorHistory.size() > 30) {  // Keep more history
            model.errorHistory.pop_front();
        }
        
        // Enhanced uncertainty estimation
        if (model.errorHistory.size() > 5) {
            // Basic statistics
            double mean = 0;
            for (double e : model.errorHistory) {
                mean += e;
            }
            mean /= model.errorHistory.size();
            
            double variance = 0;
            for (double e : model.errorHistory) {
                variance += (e - mean) * (e - mean);
            }
            variance /= model.errorHistory.size();
            
            // Calculate trend (improving or worsening)
            double trend = 0;
            if (model.errorHistory.size() > 10) {
                double early_mean = 0;
                double late_mean = 0;
                size_t mid = model.errorHistory.size() / 2;
                
                for (size_t i = 0; i < mid; i++) {
                    early_mean += model.errorHistory[i];
                }
                early_mean /= mid;
                
                for (size_t i = mid; i < model.errorHistory.size(); i++) {
                    late_mean += model.errorHistory[i];
                }
                late_mean /= (model.errorHistory.size() - mid);
                
                // Positive trend means errors are increasing (bad)
                trend = (late_mean - early_mean) / (early_mean + 0.001);
            }
            
            // Calculate consistency (low variance = consistent)
            double consistency = 1.0 / (1.0 + variance);
            
            // Combined uncertainty metric
            double raw_uncertainty = sqrt(variance);
            double trend_penalty = 1.0 + max(0.0, trend);  // Penalize worsening trends
            double consistency_bonus = 0.5 + 0.5 * consistency;  // Reward consistency
            
            // Update uncertainty with advanced metric
            double new_uncertainty = raw_uncertainty * trend_penalty / consistency_bonus;
            model.uncertainty = 0.85 * model.uncertainty + 0.15 * new_uncertainty;
            
            // Adaptive weight adjustment based on performance
            if (conf.useWeightedCombination) {
                // Models with low uncertainty get higher weight
                double performance_score = 1.0 / (1.0 + model.uncertainty);
                double target_weight = performance_score / (horizon + 1.0);  // Prefer shorter horizons
                model.weight = 0.95 * model.weight + 0.05 * target_weight;
            }
        }
    }
    
    // Normalize weights
    if (conf.useWeightedCombination) {
        double sum_weights = 0;
        for (const ForwardModel& model : models) {
            sum_weights += model.weight;
        }
        if (sum_weights > 0) {
            for (ForwardModel& model : models) {
                model.weight /= sum_weights;
            }
        }
    }
}

int AHSox::selectBestHorizon() const {
    int bestHorizon = minHorizon;
    double minUncertainty = 1e10;
    
    for (int horizon = minHorizon; horizon <= maxHorizon; horizon++) {
        int idx = horizon - minHorizon;
        if (idx < 0 || idx >= static_cast<int>(models.size())) continue;
        
        const ForwardModel& model = models[idx];
        if (model.uncertainty < minUncertainty) {
            minUncertainty = model.uncertainty;
            bestHorizon = horizon;
        }
    }
    
    return bestHorizon;
}

void AHSox::updateHorizonWeights() {
    // Calculate weights based on inverse uncertainty
    double totalWeight = 0;
    
    for (int idx = 0; idx < static_cast<int>(models.size()); idx++) {
        ForwardModel& model = models[idx];
        
        // Weight is inverse of uncertainty
        double newWeight = 1.0 / (model.uncertainty + 0.01);
        
        // Apply threshold - if uncertainty too high, reduce weight significantly
        if (model.uncertainty > uncertaintyThreshold) {
            newWeight *= 0.1;
        }
        
        // Smooth weight update
        model.weight += (newWeight - model.weight) * horizonAdaptRate;
        totalWeight += model.weight;
    }
    
    // Normalize weights
    if (totalWeight > 0) {
        for (ForwardModel& model : models) {
            model.weight /= totalWeight;
        }
    }
}

void AHSox::learnModels() {
    for (int horizon = minHorizon; horizon <= maxHorizon; horizon++) {
        if (t < horizon + 1) continue;
        
        int idx = horizon - minHorizon;
        if (idx < 0 || idx >= static_cast<int>(models.size())) continue;
        
        ForwardModel& model = models[idx];
        
        // Get time-loop error
        Matrix xi = calculateTLE(horizon);
        
        // Get past values
        const Matrix& y_past = y_buffer.get(-horizon);
        const Matrix& x_past = x_buffer.get(-horizon);
        
        // Update forward model
        model.A += (xi * (y_past ^ T) * epsA).mapP(0.1, clip);
        model.b += (xi * (epsA * conf.factorb)).mapP(0.1, clip);
        
        if (conf.useExtendedModel) {
            model.S += (xi * (x_past ^ T) * (epsA * conf.factorS)).mapP(0.1, clip);
        }
        
        // Apply damping
        if (damping > 0) {
            model.A *= (1.0 - damping);
            model.b *= (1.0 - damping);
            if (conf.useExtendedModel) {
                model.S *= (1.0 - damping);
            }
        }
    }
}

void AHSox::learnController() {
    if (conf.useWeightedCombination) {
        // Weighted combination of gradients from multiple horizons
        Matrix totalGradientC(number_motors, number_sensors);
        Matrix totalGradientH(number_motors, 1);
        totalGradientC.toZero();
        totalGradientH.toZero();
        
        for (int horizon = minHorizon; horizon <= maxHorizon; horizon++) {
            int idx = horizon - minHorizon;
            if (idx < 0 || idx >= static_cast<int>(models.size())) continue;
            
            const ForwardModel& model = models[idx];
            if (model.weight < 0.01) continue;  // Skip low-weight models
            
            // Calculate gradient for this horizon
            Matrix xi = calculateTLE(horizon);
            const Matrix& x_past = x_buffer.get(-horizon);
            const Matrix& y_past = y_buffer.get(-horizon);
            
            // Calculate Jacobian for this model
            Matrix z = C * x_past + h;
            Matrix g_prime = z.map(g_s);
            Matrix L_h = model.A * C.multrowwise(g_prime);
            if (conf.useExtendedModel) {
                L_h += model.S;
            }
            
            // Calculate homeokinetic gradient
            Matrix Lplus = L_h.pseudoInverse();
            Matrix v = Lplus * xi;
            Matrix chi = (Lplus ^ T) * v;
            Matrix mu = ((model.A ^ T) & g_prime) * chi;
            
            // Anti-Hebbian term
            Matrix epsrel = (mu & (C * v)) * (sense * 2);
            
            // Accumulate weighted gradients
            totalGradientC += ((mu * (v ^ T) - epsrel.multrowwise(y_past) * (x_past ^ T)) * model.weight);
            totalGradientH += ((mu - epsrel.multrowwise(y_past)) * model.weight);
        }
        
        // Apply combined update
        C += (totalGradientC * epsC).mapP(0.05, clip);
        h += (totalGradientH * (epsC * conf.factorh)).mapP(0.05, clip);
        
    } else {
        // Single horizon update
        int idx = static_cast<int>(currentHorizon) - minHorizon;
        if (idx >= 0 && idx < static_cast<int>(models.size())) {
            const ForwardModel& model = models[idx];
            
            Matrix xi = calculateTLE(static_cast<int>(currentHorizon));
            const Matrix& x_past = x_buffer.get(-static_cast<int>(currentHorizon));
            const Matrix& y_past = y_buffer.get(-static_cast<int>(currentHorizon));
            
            // Standard Sox learning rule with selected horizon
            Matrix z = C * x_past + h;
            Matrix g_prime = z.map(g_s);
            L = model.A * C.multrowwise(g_prime);
            if (conf.useExtendedModel) {
                L += model.S;
            }
            
            Matrix Lplus = L.pseudoInverse();
            Matrix v = Lplus * xi;
            Matrix chi = (Lplus ^ T) * v;
            Matrix mu = ((model.A ^ T) & g_prime) * chi;
            Matrix epsrel = (mu & (C * v)) * (sense * 2);
            
            C += ((mu * (v ^ T) - epsrel.multrowwise(y_past) * (x_past ^ T)) * epsC).mapP(0.05, clip);
            h += ((mu - epsrel.multrowwise(y_past)) * (epsC * conf.factorh)).mapP(0.05, clip);
        }
    }
}

double AHSox::getUncertainty(int horizon) const {
    int idx = horizon - minHorizon;
    if (idx >= 0 && idx < static_cast<int>(models.size())) {
        return models[idx].uncertainty;
    }
    return -1;
}

double AHSox::getAverageUncertainty() const {
    if (models.empty()) return 0.0;
    
    double sum = 0.0;
    for (const auto& model : models) {
        sum += model.uncertainty;
    }
    return sum / models.size();
}

Matrix AHSox::getBufferedSensor(int offset) const {
    if (offset > 0 || -offset >= buffersize) {
        return Matrix(number_sensors, 1);  // Return zero matrix
    }
    return x_buffer.get(offset);
}

Matrix AHSox::getBufferedMotor(int offset) const {
    if (offset > 0 || -offset >= buffersize) {
        return Matrix(number_motors, 1);  // Return zero matrix
    }
    return y_buffer.get(offset);
}

bool AHSox::store(FILE* f) const {
    // Store configuration
    fwrite(&conf, sizeof(conf), 1, f);
    
    // Store matrices
    C.store(f);
    h.store(f);
    
    // Store models
    int numModels = models.size();
    fwrite(&numModels, sizeof(int), 1, f);
    
    for (const ForwardModel& model : models) {
        model.A.store(f);
        model.S.store(f);
        model.b.store(f);
        fwrite(&model.uncertainty, sizeof(double), 1, f);
        fwrite(&model.weight, sizeof(double), 1, f);
    }
    
    // Store current state
    int currentHorizonInt = static_cast<int>(currentHorizon);
    fwrite(&currentHorizonInt, sizeof(int), 1, f);
    fwrite(&t, sizeof(int), 1, f);
    
    Configurable::print(f, 0);
    return true;
}

bool AHSox::restore(FILE* f) {
    // Restore configuration
    fread(&conf, sizeof(conf), 1, f);
    
    // Restore matrices
    C.restore(f);
    h.restore(f);
    
    // Restore models
    int numModels;
    fread(&numModels, sizeof(int), 1, f);
    
    models.clear();
    for (int i = 0; i < numModels; i++) {
        ForwardModel model;
        model.A.restore(f);
        model.S.restore(f);
        model.b.restore(f);
        fread(&model.uncertainty, sizeof(double), 1, f);
        fread(&model.weight, sizeof(double), 1, f);
        models.push_back(model);
    }
    
    // Restore current state
    int currentHorizonInt;
    fread(&currentHorizonInt, sizeof(int), 1, f);
    currentHorizon = static_cast<double>(currentHorizonInt);
    fread(&t, sizeof(int), 1, f);
    
    Configurable::parse(f);
    
    // Reset buffers - fill with zero matrices
    matrix::Matrix zero_matrix(number_sensors > 0 ? number_sensors : 1, 1);
    zero_matrix.toZero();
    x_buffer.fill(zero_matrix);
    
    zero_matrix.set(number_motors > 0 ? number_motors : 1, 1);
    zero_matrix.toZero();
    y_buffer.fill(zero_matrix);
    
    return true;
}