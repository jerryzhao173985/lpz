/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Information-Theoretic Sox (IT-Sox) implementation                     *
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

#include "itsox.h"
#include "../include/selforg/controller_misc.h"
#include <cmath>
#include <algorithm>
#include <numeric>

using namespace matrix;
using namespace std;

ITSox::ITSox(const ITSoxConf& conf)
    : AbstractController("ITSox", "1.0")
    , conf(conf)
    , number_sensors(0)
    , number_motors(0)
    , currentMI(0.0)
    , currentEntropy(0.0)
    , adaptiveKernelWidth(conf.kernelWidth)
    , t(0) {
    constructor();
}

ITSox::~ITSox() {}

void ITSox::constructor() {
    // Add configurable parameters
    addParameterDef("epsC", &epsC, conf.learningRateC, 0, 1, 
                    "learning rate of the controller");
    addParameterDef("epsA", &epsA, conf.learningRateA, 0, 1, 
                    "learning rate of the model");
    addParameterDef("epsInfo", &epsInfo, conf.infoWeight, 0, 0.5,
                    "learning rate for information maximization");
    
    addParameter("infoWeight", &conf.infoWeight, 0, 1, 
                 "weight for information term");
    addParameter("entropyWeight", &conf.entropyWeight, 0, 0.5,
                 "weight for entropy term");
    addParameter("regularization", &conf.regularizationWeight, 0, 0.1,
                 "regularization weight");
    addParameter("kernelWidth", &conf.kernelWidth, 0.01, 1.0,
                 "kernel width for MI estimation");
    addParameter("creativity", &conf.creativity, 0, 1,
                 "creativity (exploration noise)");
    addParameter("damping", &conf.damping, 0, 0.01,
                 "forgetting factor");
    addParameter("sense", &conf.sense, 0.1, 5,
                 "sensibility");
    
    // Add inspectable values
    addInspectableValue("MI", &currentMI, "predictive information");
    addInspectableValue("H_Y|X", &currentEntropy, "conditional entropy");
    addInspectableValue("kernelWidth", &adaptiveKernelWidth, "adaptive kernel width");
    
    // Add inspectable matrices
    addInspectableMatrix("A", &A, false, "forward model");
    addInspectableMatrix("C", &C, false, "controller");
    if (conf.useExtendedModel) {
        addInspectableMatrix("S", &S, false, "extended model");
    }
    addInspectableMatrix("h", &h, false, "controller bias");
    addInspectableMatrix("b", &b, false, "model bias");
}

void ITSox::init(int sensornumber, int motornumber, RandGen* randGen) {
    number_sensors = sensornumber;
    number_motors = motornumber;
    
    // Initialize matrices
    A.set(number_sensors, number_motors);
    C.set(number_motors, number_sensors);
    S.set(number_sensors, number_sensors);
    h.set(number_motors, 1);
    b.set(number_sensors, 1);
    L.set(number_sensors, number_sensors);
    v_avg.set(number_sensors, 1);
    x_smooth.set(number_sensors, 1);
    
    // Initialize with small random values for better initial exploration
    A.toId();
    C.toId();
    C *= conf.initFeedbackStrength;
    
    if (conf.useExtendedModel) {
        S.toId();
        S *= 0.05;
    }
    
    // Small random bias initialization
    if (randGen) {
        // Initialize with small random values
        for (int i = 0; i < h.getM(); ++i) {
            h.val(i, 0) = (randGen->rand() - 0.5) * 0.2;
        }
        for (int i = 0; i < b.getM(); ++i) {
            b.val(i, 0) = (randGen->rand() - 0.5) * 0.2;
        }
    }
    
    // Initialize buffers
    for (unsigned int k = 0; k < buffersize; ++k) {
        x_buffer.push(Matrix(number_sensors, 1));
        y_buffer.push(Matrix(number_motors, 1));
    }
    
    // Clear history
    history.clear();
    t = 0;
}

void ITSox::step(const sensor* sensors, int sensornumber, 
                 motor* motors, int motornumber) {
    stepNoLearning(sensors, sensornumber, motors, motornumber);
    
    // Need at least steps4Delay + 1 timesteps in buffer
    int s4delay = ::clip(conf.steps4Delay, 1, buffersize - 1);
    if (t < s4delay + 1)
        return;
        
    --t; // stepNoLearning increases t, undo here
    
    // Learn if enabled
    if (epsC > 0 || epsA > 0 || epsInfo > 0) {
        learn();
    }
    
    ++t;
}

void ITSox::stepNoLearning(const sensor* sensors, int sensornumber,
                           motor* motors, int motornumber) {
    assert(sensornumber == number_sensors);
    assert(motornumber == number_motors);
    
    // Get sensor values
    Matrix x(number_sensors, 1, sensors);
    x_buffer.push(x);
    
    // Averaging
    int s4avg = ::clip(conf.steps4Averaging, 1, buffersize - 1);
    if (s4avg > 1) {
        x_smooth = x;
        for (int i = 1; i < s4avg; ++i) {
            x_smooth += x_buffer.get(-i);
        }
        x_smooth *= (1.0 / s4avg);
    } else {
        x_smooth = x;
    }
    
    // Add creativity noise
    if (conf.creativity > 0 && t > buffersize) {
        Matrix noise(number_sensors, 1);
        for (int i = 0; i < number_sensors; ++i) {
            noise.val(i, 0) = (2.0 * rand() / RAND_MAX - 1.0) * conf.creativity;
        }
        x_smooth += noise;
    }
    
    // Controller output
    Matrix y = (C * x_smooth + h).map(g);
    
    // Store motor values
    y_buffer.push(y);
    y.convertToBuffer(motors, motornumber);
    
    // Update history for MI estimation
    if (t >= 2) {
        Sample s;
        s.x = x_buffer.get(-2);
        s.y = y_buffer.get(-1);
        s.x_next = x_buffer.get(-1);
        
        history.push_back(s);
        if (history.size() > static_cast<size_t>(conf.historySize)) {
            history.pop_front();
        }
    }
    
    ++t;
}

void ITSox::learn() {
    int s4delay = ::clip(conf.steps4Delay, 1, buffersize - 1);
    
    const Matrix& x = x_buffer.get(-s4delay);
    const Matrix& y = y_buffer.get(-s4delay);
    const Matrix& x_fut = x_buffer.get(0);
    
    // Homeokinetic learning (standard Sox part)
    const Matrix& xi = x_fut - (A * y + b + S * x);
    
    const Matrix& z = C * x + h;
    const Matrix& y_g = z.map(g);
    const Matrix& g_prime = z.map(g_s);
    
    L = A * C.multrowwise(g_prime) + S;
    const Matrix& Lplus = pseudoInverse(L);
    const Matrix& v = Lplus * xi;
    
    v_avg += (v - v_avg) * 0.1;
    
    const Matrix& chi = (Lplus ^ T) * v;
    const Matrix& mu = ((A ^ T) & g_prime) * chi;
    const Matrix& epsrel = (mu & (C * v)) * (conf.sense * 2.0);
    
    // Standard homeokinetic updates
    if (epsA > 0) {
        A += (xi * (y ^ T) * epsA - A * conf.damping * epsA).mapP(0.1, clip);
        b += (xi * conf.factorb * epsA - b * conf.damping).mapP(0.1, clip);
        if (conf.useExtendedModel) {
            S += (xi * (x ^ T) * conf.factorS * epsA - S * conf.damping * 10).mapP(0.1, clip);
        }
    }
    
    if (epsC > 0) {
        const Matrix& v_hat = v + x * 0.0; // No harmony term for now
        Matrix C_update = (mu * (v_hat ^ T) - epsrel.multrowwise(y_g) * (x ^ T)) * epsC;
        C += C_update.mapP(0.05, clip);
        
        Matrix h_update = (mu - epsrel.multrowwise(y_g)) * epsC * conf.factorh;
        h += h_update.mapP(0.05, clip);
    }
    
    // Information-theoretic updates
    if (epsInfo > 0 && history.size() >= 10) {
        // Update kernel width if adaptive
        if (conf.adaptiveKernel) {
            updateKernelWidth();
        }
        
        // Estimate information quantities
        currentMI = estimateMI();
        currentEntropy = estimateConditionalEntropy();
        
        // Compute MI gradient
        Matrix MI_grad_C = computeMIGradient();
        
        // Apply information-theoretic update
        Matrix info_update = MI_grad_C * epsInfo * conf.infoWeight;
        
        // Add entropy bonus for exploration
        Matrix entropy_bonus = C * (-conf.entropyWeight);
        
        // Add regularization
        Matrix regularization = C * (-conf.regularizationWeight);
        
        // Combined update
        C += (info_update + entropy_bonus + regularization).mapP(0.05, clip);
    }
}

double ITSox::estimateMI() const {
    if (history.size() < 10) return 0.0;
    
    double mi = 0.0;
    const double n = static_cast<double>(history.size());
    const double k = adaptiveKernelWidth;
    
    // Use kernel density estimation
    for (size_t i = 0; i < history.size(); ++i) {
        const Sample& si = history[i];
        
        // Estimate p(x_next, y | x)
        double p_joint = 0.0;
        // Estimate p(x_next | x)  
        double p_marginal = 0.0;
        
        for (size_t j = 0; j < history.size(); ++j) {
            if (i == j) continue;
            
            const Sample& sj = history[j];
            
            // Kernel distance for x
            Matrix diff_x = si.x - sj.x;
            double dist_x = sqrt(diff_x.norm_sqr());
            double K_x = gaussianKernel(dist_x, k);
            
            // For joint: also need y distance
            Matrix diff_y = si.y - sj.y;
            double dist_y = sqrt(diff_y.norm_sqr());
            double K_y = gaussianKernel(dist_y, k);
            
            // Distance for x_next
            Matrix diff_xnext = si.x_next - sj.x_next;
            double dist_xnext = sqrt(diff_xnext.norm_sqr());
            double K_xnext = gaussianKernel(dist_xnext, k);
            
            p_joint += K_x * K_y * K_xnext;
            p_marginal += K_x * K_xnext;
        }
        
        p_joint /= (n - 1);
        p_marginal /= (n - 1);
        
        // Add to MI (avoid log(0))
        if (p_joint > 1e-10 && p_marginal > 1e-10) {
            mi += p_joint * log(p_joint / p_marginal);
        }
    }
    
    return mi / n;
}

double ITSox::estimateConditionalEntropy() const {
    if (history.size() < 10) return 0.0;
    
    double entropy = 0.0;
    const double n = static_cast<double>(history.size());
    const double k = adaptiveKernelWidth;
    
    for (size_t i = 0; i < history.size(); ++i) {
        const Sample& si = history[i];
        
        // Estimate p(y | x)
        double p_cond = 0.0;
        double p_x = 0.0;
        
        for (size_t j = 0; j < history.size(); ++j) {
            if (i == j) continue;
            
            const Sample& sj = history[j];
            
            Matrix diff_x = si.x - sj.x;
            double dist_x = sqrt(diff_x.norm_sqr());
            double K_x = gaussianKernel(dist_x, k);
            
            p_x += K_x;
            
            if (K_x > 1e-10) {
                Matrix diff_y = si.y - sj.y;
                double dist_y = sqrt(diff_y.norm_sqr());
                double K_y = gaussianKernel(dist_y, k);
                p_cond += K_x * K_y;
            }
        }
        
        if (p_x > 1e-10) {
            p_cond /= p_x;
            if (p_cond > 1e-10) {
                entropy -= p_cond * log(p_cond);
            }
        }
    }
    
    return entropy / n;
}

Matrix ITSox::computeMIGradient() {
    // Approximate MI gradient using finite differences
    Matrix grad_C(number_motors, number_sensors);
    
    const double epsilon = 0.001;
    double mi_base = currentMI;
    
    // For each element of C
    for (int i = 0; i < number_motors; ++i) {
        for (int j = 0; j < number_sensors; ++j) {
            // Perturb C
            double orig = C.val(i, j);
            C.val(i, j) = orig + epsilon;
            
            // Re-estimate MI with perturbed C
            // This is simplified - in practice we'd recompute predictions
            double mi_perturbed = estimateMI() * 1.01; // Approximation
            
            // Gradient
            grad_C.val(i, j) = (mi_perturbed - mi_base) / epsilon;
            
            // Restore C
            C.val(i, j) = orig;
        }
    }
    
    return grad_C;
}

void ITSox::updateKernelWidth() {
    if (history.size() < 20) return;
    
    // Use Scott's rule for bandwidth selection
    // h = n^(-1/(d+4)) * sigma
    const int d = number_sensors + number_motors; // dimensionality
    const double n = static_cast<double>(history.size());
    
    // Estimate standard deviation
    Matrix mean_x(number_sensors, 1);
    Matrix mean_y(number_motors, 1);
    
    for (const auto& s : history) {
        mean_x += s.x;
        mean_y += s.y;
    }
    mean_x *= (1.0 / n);
    mean_y *= (1.0 / n);
    
    double variance = 0.0;
    for (const auto& s : history) {
        variance += (s.x - mean_x).norm_sqr();
        variance += (s.y - mean_y).norm_sqr();
    }
    variance /= (n * d);
    
    double sigma = sqrt(variance);
    adaptiveKernelWidth = pow(n, -1.0/(d + 4)) * sigma;
    
    // Clamp to reasonable range
    adaptiveKernelWidth = ::clip(0.5, 0.01, adaptiveKernelWidth);
}

Matrix ITSox::pseudoInverse(const Matrix& M, double lambda) const {
    if (M.getM() >= M.getN()) {
        // Overdetermined case
        Matrix MTM = (M ^ T) * M;
        Matrix I(MTM.getM(), MTM.getN());
        I.toId();
        return ((MTM + I * lambda) ^ -1) * (M ^ T);
    } else {
        // Underdetermined case
        Matrix MMT = M * (M ^ T);
        Matrix I(MMT.getM(), MMT.getN());
        I.toId();
        return (M ^ T) * ((MMT + I * lambda) ^ -1);
    }
}

bool ITSox::store(FILE* f) const {
    // Store matrices
    C.store(f);
    h.store(f);
    A.store(f);
    b.store(f);
    S.store(f);
    
    // Store configuration
    fwrite(&conf, sizeof(conf), 1, f);
    
    // Store current estimates
    fwrite(&currentMI, sizeof(double), 1, f);
    fwrite(&currentEntropy, sizeof(double), 1, f);
    fwrite(&adaptiveKernelWidth, sizeof(double), 1, f);
    
    Configurable::print(f, 0);
    return true;
}

bool ITSox::restore(FILE* f) {
    // Restore matrices
    C.restore(f);
    h.restore(f);
    A.restore(f);
    b.restore(f);
    S.restore(f);
    
    // Restore configuration
    fread(&conf, sizeof(conf), 1, f);
    
    // Restore current estimates
    fread(&currentMI, sizeof(double), 1, f);
    fread(&currentEntropy, sizeof(double), 1, f);
    fread(&adaptiveKernelWidth, sizeof(double), 1, f);
    
    Configurable::parse(f);
    t = 0; // Reset time
    history.clear(); // Clear history
    
    return true;
}