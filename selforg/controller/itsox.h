/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
 *                                                                         *
 *   Information-Theoretic Sox (IT-Sox) - A homeokinetic controller that  *
 *   directly optimizes predictive information                             *
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
#ifndef __ITSOX_H
#define __ITSOX_H

#include "abstractcontroller.h"
#include "controller_misc.h"
#include "configurable.h"
#include "matrix.h"
#include "circular_buffer.h"
#include <vector>
#include <deque>
#include <cmath>

/**
 * Information-Theoretic Sox (IT-Sox)
 * 
 * This controller extends the Sox algorithm with direct optimization of
 * predictive information I(X_{t+1}; Y_t | X_t). It combines the robustness
 * of homeokinetic control with information-theoretic objectives.
 * 
 * Key innovations:
 * - Direct optimization of mutual information
 * - Kernel density estimation for continuous variables
 * - Adaptive balance between prediction and exploration
 * - Information-driven learning rates
 * 
 * The controller optimizes:
 * E = -I(X_{t+1}; Y_t | X_t) + β·H(Y|X) - γ·||C||²
 * 
 * Where:
 * - I(...) is the predictive information (maximized)
 * - H(Y|X) is the conditional entropy (maintains exploration)
 * - ||C||² is the regularization term
 */
class ITSox : public AbstractController {
public:
    /// Configuration structure
    struct ITSoxConf {
        double initFeedbackStrength;    ///< initial strength of controller matrix C
        bool useExtendedModel;          ///< use extended model with S matrix
        
        // Information-theoretic parameters
        double infoWeight;              ///< weight for information term (default: 0.1)
        double entropyWeight;           ///< weight for entropy term (default: 0.05)
        double regularizationWeight;    ///< weight for regularization (default: 0.001)
        int historySize;                ///< samples for MI estimation (default: 100)
        double kernelWidth;             ///< kernel width for KDE (default: 0.1)
        bool adaptiveKernel;            ///< use adaptive kernel width (default: true)
        
        // Standard homeokinetic parameters
        double learningRateC;           ///< learning rate for controller
        double learningRateA;           ///< learning rate for forward model
        double damping;                 ///< forgetting factor
        double creativity;              ///< exploration noise
        double sense;                   ///< sensibility parameter
        
        // Extended model factors
        double factorS;                 ///< factor for S learning rate
        double factorb;                 ///< factor for b learning rate  
        double factorh;                 ///< factor for h learning rate
        
        // Control parameters
        int steps4Averaging;            ///< sensor averaging steps
        int steps4Delay;                ///< motor delay steps
    };
    
    /// Returns default configuration
    static ITSoxConf getDefaultConf() {
        ITSoxConf conf;
        conf.initFeedbackStrength = 1.0;
        conf.useExtendedModel = false;
        
        // Information-theoretic parameters
        conf.infoWeight = 0.1;
        conf.entropyWeight = 0.05;
        conf.regularizationWeight = 0.001;
        conf.historySize = 100;
        conf.kernelWidth = 0.1;
        conf.adaptiveKernel = true;
        
        // Homeokinetic parameters
        conf.learningRateC = 0.1;
        conf.learningRateA = 0.1;
        conf.damping = 0.00001;
        conf.creativity = 0.0;
        conf.sense = 1.0;
        
        // Extended model
        conf.factorS = 1.0;
        conf.factorb = 0.1;
        conf.factorh = 1.0;
        
        // Control
        conf.steps4Averaging = 1;
        conf.steps4Delay = 1;
        
        return conf;
    }
    
    /// Constructor
    explicit ITSox(const ITSoxConf& conf = getDefaultConf());
    
    /// Destructor
    virtual ~ITSox();
    
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
    
    /// Stores the controller to file
    virtual bool store(FILE* f) const;
    
    /// Restores the controller from file
    virtual bool restore(FILE* f);
    
    /// Returns current predictive information
    double getPredictiveInformation() const { return currentMI; }
    
    /// Returns current entropy H(Y|X)
    double getConditionalEntropy() const { return currentEntropy; }
    
protected:
    ITSoxConf conf;                     ///< Configuration
    unsigned short number_sensors;       ///< Number of sensors
    unsigned short number_motors;        ///< Number of motors
    
    // Core matrices (same as Sox)
    matrix::Matrix A;                    ///< Forward model matrix
    matrix::Matrix C;                    ///< Controller matrix
    matrix::Matrix S;                    ///< Extended model matrix
    matrix::Matrix h;                    ///< Controller bias
    matrix::Matrix b;                    ///< Model bias
    matrix::Matrix L;                    ///< Jacobian matrix
    
    // Buffers
    static constexpr unsigned short buffersize = 50;
    lpzrobots::CircularBuffer<matrix::Matrix, buffersize> x_buffer;
    lpzrobots::CircularBuffer<matrix::Matrix, buffersize> y_buffer;
    
    // Information estimation
    struct Sample {
        matrix::Matrix x;    // sensor values
        matrix::Matrix y;    // motor values
        matrix::Matrix x_next; // next sensor values
    };
    std::deque<Sample> history;         ///< History for MI estimation
    
    // Current estimates
    double currentMI;                   ///< Current mutual information
    double currentEntropy;              ///< Current conditional entropy
    double adaptiveKernelWidth;         ///< Current kernel width
    
    // Internal state
    matrix::Matrix x_smooth;            ///< Smoothed sensor values
    matrix::Matrix v_avg;               ///< Average TLE in motor space
    int t;                              ///< Time step counter
    
    // Configurable parameters
    paramval epsC;                      ///< Controller learning rate
    paramval epsA;                      ///< Model learning rate
    paramval epsInfo;                   ///< Information learning rate
    
    /// Initialize parameters
    void constructor();
    
    /// Learn controller and model with information maximization
    virtual void learn();
    
    /// Estimate mutual information using kernel density estimation
    double estimateMI() const;
    
    /// Estimate conditional entropy H(Y|X)
    double estimateConditionalEntropy() const;
    
    /// Compute MI gradient w.r.t controller parameters
    matrix::Matrix computeMIGradient();
    
    /// Gaussian kernel for KDE
    static double gaussianKernel(double distance, double width) {
        return std::exp(-0.5 * distance * distance / (width * width));
    }
    
    /// Update adaptive kernel width based on data
    void updateKernelWidth();
    
    /// Neuron transfer function (tanh)
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
    
    /// Compute pseudoinverse with regularization
    matrix::Matrix pseudoInverse(const matrix::Matrix& M, double lambda = 0.01) const;
};

#endif // __ITSOX_H