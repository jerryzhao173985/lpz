/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Context-Aware Sox (CA-Sox) Implementation                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "casox.h"
#include <algorithm>
#include <cmath>
#include <numeric>

using namespace matrix;
using namespace std;

CASox::CASox(const CASoxConf& conf)
    : AbstractController("CASox", "1.0")
    , conf(conf)
    , maxContexts(conf.maxContexts)
    , contextRadius(conf.contextRadius)
    , contextThreshold(conf.contextThreshold)
    , contextAdaptRate(conf.contextAdaptRate)
    , radiusAdaptRate(conf.radiusAdaptRate)
    , number_sensors(0)
    , number_motors(0)
    , buffersize(50)
    , t(0)
    , currentContext(-1) {
    
    // Add configurable parameters
    addParameterDef("epsC", &epsC, conf.learningRateC, 0, 5, "learning rate of the controller");
    addParameterDef("epsA", &epsA, conf.learningRateA, 0, 5, "learning rate of the models");
    addParameterDef("epsGate", &epsGate, conf.gateLearningRate, 0, 5, "learning rate of gating");
    addParameterDef("creativity", &creativity, conf.creativity, 0, 1, "creativity term");
    addParameterDef("damping", &damping, conf.damping, 0, 0.01, "forgetting term");
    
    addParameter("maxContexts", &maxContexts, 1, 20, "maximum number of contexts");
    addParameter("contextRadius", &contextRadius, 0.01, 2.0, "context detection radius");
    addParameter("contextThreshold", &contextThreshold, 0, 1, "threshold for new context");
    addParameter("contextAdaptRate", &contextAdaptRate, 0, 1, "context adaptation rate");
    addParameter("radiusAdaptRate", &radiusAdaptRate, 0, 0.1, "radius adaptation rate");
    
    // Inspectable values
    addInspectableValue("numContexts", &currentContext, "number of active contexts");
    addInspectableValue("currentContext", &currentContext, "currently active context");
    addInspectableMatrix("C", &C, false, "controller matrix");
    addInspectableMatrix("h", &h, false, "controller bias");
}

CASox::~CASox() {}

void CASox::init(int sensornumber, int motornumber, RandGen* randGen) {
    assert(sensornumber > 0 && motornumber > 0);
    
    number_sensors = sensornumber;
    number_motors = motornumber;
    
    // Initialize controller matrices
    C = Matrix(number_motors, number_sensors);
    h = Matrix(number_motors, 1);
    L = Matrix(number_sensors, number_motors);
    
    C.toId();
    C *= conf.initFeedbackStrength;
    h.toZero();
    
    // Clear contexts
    contexts.clear();
    currentContext = -1;
    
    // Initialize buffers
    for (int i = 0; i < buffersize; i++) {
        x_buffer.push(Matrix(number_sensors, 1));
        y_buffer.push(Matrix(number_motors, 1));
    }
    
    x_smooth = Matrix(number_sensors, 1);
    x_smooth.toZero();
    
    t = 0;
}

void CASox::stepNoLearning(const sensor* x_, int number_sensors_robot,
                           motor* y_, int number_motors_robot) {
    assert(number_sensors_robot <= number_sensors && number_motors_robot <= number_motors);
    
    // Store sensor values
    Matrix x(number_sensors, 1);
    x.set(number_sensors_robot, 1, x_);
    
    // Smooth sensor values
    x_smooth += (x - x_smooth) * 0.1;
    
    // Store in buffer
    x_buffer.push(x_smooth);
    
    // Detect or create context
    if (contexts.empty()) {
        createNewContext(x_smooth);
    }
    
    currentContext = static_cast<double>(detectContext(x_smooth));
    
    // Update context activations
    updateContextActivations(x_smooth);
    
    // Calculate motor values
    Matrix y;
    
    // Handle creativity properly
    if (creativity > 0 && currentContext >= 0 && static_cast<int>(currentContext) < static_cast<int>(contexts.size())) {
        const ContextModel& context = contexts[static_cast<int>(currentContext)];
        
        // Use context-specific exploration level
        double contextCreativity = creativity;
        if (conf.contextSpecificExploration) {
            contextCreativity *= context.explorationLevel;
        }
        
        // First compute output without creativity for prediction
        Matrix y_temp = (C * x_smooth + h).map(g);
        
        // Predict next state using context model
        Matrix x_pred = context.A * y_temp + context.b;
        if (conf.useExtendedModel) {
            x_pred += context.S * x_smooth;
        }
        
        // Apply creativity to input, not output
        Matrix x_creative = x_smooth + (x_pred - x_smooth) * contextCreativity;
        y = (C * x_creative + h).map(g);
    } else {
        y = (C * x_smooth + h).map(g);
    }
    
    // Store in buffer
    y_buffer.push(y);
    
    // Convert to motor array
    y.convertToBuffer(y_, number_motors_robot);
    
    t++;
}

void CASox::step(const sensor* x_, int number_sensors_robot,
                motor* y_, int number_motors_robot) {
    // First do the step without learning
    stepNoLearning(x_, number_sensors_robot, y_, number_motors_robot);
    
    // Only start learning after initial buffer fill
    if (t < 3) return;
    
    // Update context statistics
    updateContextStatistics();
    
    // Learn context models
    if (epsA > 0) {
        learnContextModels();
    }
    
    // Learn controller
    if (epsC > 0) {
        learnController();
    }
    
    // Adapt context prototypes
    if (conf.contextAdaptRate > 0) {
        adaptContextPrototypes();
    }
    
    // Merge similar contexts if enabled
    if (conf.dynamicContexts && contexts.size() > 1) {
        mergeContexts();
    }
}

int CASox::detectContext(const Matrix& x) {
    if (contexts.empty()) return -1;
    
    int bestContext = 0;
    double minDistance = distanceToContext(x, contexts[0]);
    
    for (size_t i = 1; i < contexts.size(); i++) {
        double dist = distanceToContext(x, contexts[i]);
        if (dist < minDistance) {
            minDistance = dist;
            bestContext = i;
        }
    }
    
    // Check if we need a new context
    if (conf.dynamicContexts && 
        minDistance > contextThreshold && 
        static_cast<int>(contexts.size()) < maxContexts) {
        createNewContext(x);
        return contexts.size() - 1;
    }
    
    return bestContext;
}

void CASox::createNewContext(const Matrix& x) {
    ContextModel newContext;
    
    // Initialize model matrices
    newContext.A = Matrix(number_sensors, number_motors);
    // For non-square matrices, initialize with small random values
    if (number_sensors != number_motors) {
        for (int i = 0; i < number_sensors; i++) {
            for (int j = 0; j < number_motors; j++) {
                newContext.A.val(i, j) = 0.1 * (drand48() - 0.5);
            }
        }
    } else {
        newContext.A.toId();
        newContext.A *= 0.1;
    }
    
    newContext.S = Matrix(number_sensors, number_sensors);
    newContext.S.toZero();
    
    newContext.b = Matrix(number_sensors, 1);
    newContext.b.toZero();
    
    // Set prototype to current sensor values
    newContext.prototype = x;
    newContext.radius = contextRadius;
    newContext.activation = 1.0;
    
    // Initialize statistics
    newContext.useCount = 0;
    newContext.avgError = 1.0;
    
    // Context-specific parameters
    newContext.explorationLevel = 1.0 + 0.2 * contexts.size();  // New contexts explore more
    newContext.learningRateFactor = 1.0;
    
    contexts.push_back(newContext);
}

void CASox::updateContextActivations(const Matrix& x) {
    contextGating = calculateGating(x);
    
    // Update activation levels
    for (size_t i = 0; i < contexts.size(); i++) {
        contexts[i].activation = contextGating[i];
    }
}

vector<double> CASox::calculateGating(const Matrix& x) {
    vector<double> distances(contexts.size());
    vector<double> gates(contexts.size());
    
    // Calculate distances
    for (size_t i = 0; i < contexts.size(); i++) {
        distances[i] = distanceToContext(x, contexts[i]);
    }
    
    // Convert to activations (inverse distance with softmax)
    double sumExp = 0;
    for (size_t i = 0; i < contexts.size(); i++) {
        gates[i] = exp(-distances[i] / contextRadius);
        sumExp += gates[i];
    }
    
    // Normalize
    if (sumExp > 0) {
        for (double& gate : gates) {
            gate /= sumExp;
        }
    }
    
    return gates;
}

double CASox::distanceToContext(const Matrix& x, const ContextModel& context) {
    // Check dimension compatibility
    if (x.getM() != context.prototype.getM() || 
        x.getN() != context.prototype.getN()) {
        // Return large distance for incompatible contexts
        return 1000.0;  
    }
    
    Matrix diff = x - context.prototype;
    return sqrt(diff.norm_sqr()) / context.radius;
}

void CASox::mergeContexts() {
    // Check all pairs of contexts for similarity
    for (size_t i = 0; i < contexts.size(); i++) {
        for (size_t j = i + 1; j < contexts.size(); j++) {
            // Check dimension compatibility first
            if (contexts[i].prototype.getM() != contexts[j].prototype.getM() ||
                contexts[i].prototype.getN() != contexts[j].prototype.getN()) {
                continue;  // Skip incompatible contexts
            }
            
            Matrix diff = contexts[i].prototype - contexts[j].prototype;
            double distance = sqrt(diff.norm_sqr());
            
            if (distance < conf.mergeThreshold) {
                // Merge context j into i
                ContextModel& ctx_i = contexts[i];
                const ContextModel& ctx_j = contexts[j];
                
                // Weighted average of prototypes
                double total_use = ctx_i.useCount + ctx_j.useCount + 2;
                double weight_i = (ctx_i.useCount + 1) / total_use;
                double weight_j = (ctx_j.useCount + 1) / total_use;
                
                ctx_i.prototype = ctx_i.prototype * weight_i + ctx_j.prototype * weight_j;
                
                // Merge models (weighted average)
                ctx_i.A = ctx_i.A * weight_i + ctx_j.A * weight_j;
                ctx_i.S = ctx_i.S * weight_i + ctx_j.S * weight_j;
                ctx_i.b = ctx_i.b * weight_i + ctx_j.b * weight_j;
                
                // Update statistics
                ctx_i.useCount += ctx_j.useCount;
                ctx_i.radius = (ctx_i.radius + ctx_j.radius) / 2;
                
                // Remove context j
                contexts.erase(contexts.begin() + j);
                
                // Update current context if needed
                if (static_cast<int>(currentContext) == static_cast<int>(j)) {
                    currentContext = static_cast<double>(i);
                } else if (static_cast<int>(currentContext) > static_cast<int>(j)) {
                    currentContext = currentContext - 1.0;
                }
                
                return;  // Only merge one pair per step
            }
        }
    }
}

void CASox::learnContextModels() {
    if (t < 3) return;  // Need at least 3 timesteps for proper learning
    
    // Get past values
    const Matrix& x_past = x_buffer.get(-2);
    const Matrix& y_past = y_buffer.get(-2);
    const Matrix& x_now = x_buffer.get(-1);
    
    // Learn models for all contexts weighted by activation
    for (size_t i = 0; i < contexts.size(); i++) {
        ContextModel& context = contexts[i];
        
        if (context.activation < 0.01) continue;  // Skip inactive contexts
        
        // Predict with this context's model
        Matrix x_pred = context.A * y_past + context.b;
        if (conf.useExtendedModel) {
            x_pred += context.S * x_past;
        }
        
        // Prediction error
        Matrix xi = x_now - x_pred;
        
        // Weighted learning based on activation
        double contextEpsA = epsA * context.activation * context.learningRateFactor;
        
        // Update model
        context.A += (xi * (y_past ^ T) * contextEpsA).mapP(0.1, clip);
        context.b += (xi * contextEpsA * 0.1).mapP(0.1, clip);
        
        if (conf.useExtendedModel) {
            context.S += (xi * (x_past ^ T) * contextEpsA).mapP(0.1, clip);
        }
        
        // Apply damping
        if (damping > 0) {
            context.A *= (1.0 - damping);
            context.b *= (1.0 - damping);
            if (conf.useExtendedModel) {
                context.S *= (1.0 - damping);
            }
        }
        
        // Update error history
        double error = xi.norm_sqr();
        context.recentErrors.push_back(error);
        if (context.recentErrors.size() > 20) {
            context.recentErrors.pop_front();
        }
    }
}

void CASox::learnController() {
    if (t < 3) return;  // Need at least 3 timesteps for proper learning
    
    // Get past values
    const Matrix& x_past = x_buffer.get(-2);
    const Matrix& y_past = y_buffer.get(-2);
    const Matrix& x_now = x_buffer.get(-1);
    
    // Make sure contextGating is updated
    if (contextGating.empty() || contextGating.size() != contexts.size()) {
        updateContextActivations(x_past);
    }
    
    // Calculate mixed prediction using gating
    Matrix x_pred = getMixedPrediction(y_past, x_past);
    
    // Time-loop error
    Matrix xi = x_now - x_pred;
    
    // Calculate mixed Jacobian
    Matrix z = C * x_past + h;
    Matrix g_prime = z.map(g_s);
    
    // For CASox, we need to properly compute the Jacobian for the sensor-motor loop
    // The key insight is that we're learning a controller that maps sensors to motors
    // and a forward model that predicts next sensors from current motors
    
    // The full loop is: x -> y = g(C*x + h) -> x_next = A*y + b + S*x
    // So dx_next/dx = A * dg(C*x+h)/dx + S = A * diag(g'(z)) * C + S
    
    // However, for non-square systems, we need the effective Jacobian
    // that captures how sensor predictions depend on motor commands
    // This is simply the forward model Jacobian: dx_next/dy = A
    
    L.toZero();
    
    // Compute weighted average of context Jacobians
    for (size_t i = 0; i < contexts.size(); i++) {
        if (contextGating[i] < 0.01) continue;
        
        // For homeokinetic learning, we use the forward model Jacobian
        // L = dx_next/dy = A (sensors x motors)
        Matrix L_context = contexts[i].A;
        
        L += L_context * contextGating[i];
    }
    
    // For non-square configurations, we need a different approach
    // The key is to work in the appropriate space
    
    if (number_sensors == number_motors) {
        // Square case: use standard homeokinetic learning
        Matrix Lplus = L.pseudoInverse();
        Matrix v = Lplus * xi;
        Matrix chi = (Lplus ^ T) * v;
        
        // Use context-specific exploration if enabled
        double effectiveSense = 1.0;
        if (conf.contextSpecificExploration && currentContext >= 0) {
            effectiveSense *= contexts[static_cast<int>(currentContext)].explorationLevel;
        }
        
        // Calculate gradients for all active contexts
        Matrix gradC(number_motors, number_sensors);
        Matrix gradH(number_motors, 1);
        gradC.toZero();
        gradH.toZero();
        
        for (size_t i = 0; i < contexts.size(); i++) {
            if (contextGating[i] < 0.01) continue;
            
            const ContextModel& context = contexts[i];
            
            Matrix A_T_scaled = (context.A ^ T).multrowwise(g_prime);
            Matrix mu_i = A_T_scaled * chi;
            
            Matrix epsrel_i = (mu_i & (C * v)) * (effectiveSense * 2.0);
            
            gradC += ((mu_i * (v ^ T) - epsrel_i.multrowwise(y_past) * (x_past ^ T)) * contextGating[i]);
            gradH += ((mu_i - epsrel_i.multrowwise(y_past)) * contextGating[i]);
        }
        
        // Update controller
        C += (gradC * epsC).mapP(0.05, clip);
        h += (gradH * epsC * 0.1).mapP(0.05, clip);
    } else {
        // Non-square case: use simplified learning
        // Project the sensor error to motor space for controller update
        
        Matrix gradC(number_motors, number_sensors);
        Matrix gradH(number_motors, 1);
        gradC.toZero();
        gradH.toZero();
        
        for (size_t i = 0; i < contexts.size(); i++) {
            if (contextGating[i] < 0.01) continue;
            
            const ContextModel& context = contexts[i];
            
            // Project sensor error to motor error
            Matrix y_error(number_motors, 1);
            if (number_sensors > number_motors) {
                // Over-sensed: least squares solution
                Matrix AtA = (context.A ^ T) * context.A;
                y_error = AtA.pseudoInverse() * ((context.A ^ T) * xi);
            } else {
                // Under-sensed: minimum norm solution
                Matrix AAt = context.A * (context.A ^ T);
                y_error = (context.A ^ T) * (AAt.pseudoInverse() * xi);
            }
            
            // Controller gradient
            Matrix delta = y_error & g_prime;
            
            // Simple anti-Hebbian term for stability
            double anti_hebb_factor = creativity > 0 ? creativity : 0.1;
            
            gradC += (delta * (x_past ^ T)) * contextGating[i] * (1.0 - anti_hebb_factor);
            gradH += delta * contextGating[i] * (1.0 - anti_hebb_factor);
        }
        
        // Update controller
        C += (gradC * epsC).mapP(0.05, clip);
        h += (gradH * epsC * 0.1).mapP(0.05, clip);
    }
}

Matrix CASox::getMixedPrediction(const Matrix& y, const Matrix& x_past) {
    Matrix x_pred(number_sensors, 1);
    x_pred.toZero();
    
    for (size_t i = 0; i < contexts.size(); i++) {
        if (contextGating.size() <= i || contextGating[i] < 0.01) continue;
        
        const ContextModel& context = contexts[i];
        Matrix pred_i = context.A * y + context.b;
        if (conf.useExtendedModel) {
            pred_i += context.S * x_past;
        }
        
        x_pred += pred_i * contextGating[i];
    }
    
    return x_pred;
}

void CASox::updateContextStatistics() {
    for (size_t i = 0; i < contexts.size(); i++) {
        ContextModel& context = contexts[i];
        
        // Update use count
        if (i == static_cast<size_t>(currentContext)) {
            context.useCount++;
        }
        
        // Update average error
        if (!context.recentErrors.empty()) {
            context.avgError = accumulate(context.recentErrors.begin(), 
                                        context.recentErrors.end(), 0.0) 
                             / context.recentErrors.size();
        }
        
        // Adapt exploration level based on error
        if (conf.contextSpecificExploration) {
            // Low error -> less exploration, high error -> more exploration
            context.explorationLevel = 0.5 + 2.0 * context.avgError;
            context.explorationLevel = min(2.0, max(0.1, context.explorationLevel));
        }
        
        // Adapt learning rate based on use frequency
        double useFreq = static_cast<double>(context.useCount) / (t + 1);
        context.learningRateFactor = 0.5 + 1.0 / (1.0 + 10.0 * useFreq);
    }
}

void CASox::adaptContextPrototypes() {
    if (currentContext < 0 || static_cast<int>(currentContext) >= static_cast<int>(contexts.size())) return;
    
    ContextModel& context = contexts[static_cast<int>(currentContext)];
    const Matrix& x_current = x_buffer.get(-1);
    
    // Ensure prototype has correct dimensions before updating
    if (context.prototype.getM() != x_current.getM() || 
        context.prototype.getN() != x_current.getN()) {
        // Resize prototype to match current sensor dimensions
        context.prototype.set(x_current.getM(), x_current.getN());
        context.prototype = x_current;  // Reset to current value
        return;  // Skip adaptation this step
    }
    
    // Move prototype towards current observation
    context.prototype += (x_current - context.prototype) * contextAdaptRate;
    
    // Adapt radius based on prediction quality
    if (context.avgError < 0.1) {
        // Good prediction -> shrink radius
        context.radius *= (1.0 - radiusAdaptRate);
    } else if (context.avgError > 0.5) {
        // Poor prediction -> expand radius
        context.radius *= (1.0 + radiusAdaptRate);
    }
    
    // Keep radius in reasonable bounds
    context.radius = min(2.0, max(0.1, context.radius));
}

bool CASox::store(FILE* f) const {
    // Store configuration
    fwrite(&conf, sizeof(conf), 1, f);
    
    // Store controller matrices
    C.store(f);
    h.store(f);
    
    // Store contexts
    int numContexts = contexts.size();
    fwrite(&numContexts, sizeof(int), 1, f);
    
    for (const ContextModel& context : contexts) {
        context.A.store(f);
        context.S.store(f);
        context.b.store(f);
        context.prototype.store(f);
        
        fwrite(&context.radius, sizeof(double), 1, f);
        fwrite(&context.activation, sizeof(double), 1, f);
        fwrite(&context.useCount, sizeof(int), 1, f);
        fwrite(&context.avgError, sizeof(double), 1, f);
        fwrite(&context.explorationLevel, sizeof(double), 1, f);
        fwrite(&context.learningRateFactor, sizeof(double), 1, f);
    }
    
    // Store state
    int currentContextInt = static_cast<int>(currentContext);
    fwrite(&currentContextInt, sizeof(int), 1, f);
    fwrite(&t, sizeof(int), 1, f);
    
    Configurable::print(f, 0);
    return true;
}

bool CASox::restore(FILE* f) {
    // Restore configuration
    fread(&conf, sizeof(conf), 1, f);
    
    // Restore controller matrices
    C.restore(f);
    h.restore(f);
    
    // Restore contexts
    int numContexts;
    fread(&numContexts, sizeof(int), 1, f);
    
    contexts.clear();
    for (int i = 0; i < numContexts; i++) {
        ContextModel context;
        context.A.restore(f);
        context.S.restore(f);
        context.b.restore(f);
        context.prototype.restore(f);
        
        fread(&context.radius, sizeof(double), 1, f);
        fread(&context.activation, sizeof(double), 1, f);
        fread(&context.useCount, sizeof(int), 1, f);
        fread(&context.avgError, sizeof(double), 1, f);
        fread(&context.explorationLevel, sizeof(double), 1, f);
        fread(&context.learningRateFactor, sizeof(double), 1, f);
        
        // Validate restored context dimensions match current setup
        if (number_sensors > 0 && number_motors > 0) {
            // Check and fix prototype dimensions if needed
            if (context.prototype.getM() != number_sensors || 
                context.prototype.getN() != 1) {
                context.prototype.set(number_sensors, 1);
                context.prototype.toZero();
            }
            // Check and fix model dimensions if needed
            if (context.A.getM() != number_sensors || 
                context.A.getN() != number_motors) {
                context.A.set(number_sensors, number_motors);
                context.A.toId();
                context.A *= 0.1;
            }
            if (context.b.getM() != number_sensors || 
                context.b.getN() != 1) {
                context.b.set(number_sensors, 1);
                context.b.toZero();
            }
            if (conf.useExtendedModel) {
                if (context.S.getM() != number_sensors || 
                    context.S.getN() != number_sensors) {
                    context.S.set(number_sensors, number_sensors);
                    context.S.toZero();
                }
            }
        }
        
        contexts.push_back(context);
    }
    
    // Restore state
    int currentContextInt;
    fread(&currentContextInt, sizeof(int), 1, f);
    currentContext = static_cast<double>(currentContextInt);
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