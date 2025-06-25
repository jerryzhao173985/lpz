/***************************************************************************
 *   Copyright (C) 2025 LpzRobots development team                          *
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
#ifndef __CONTROLLERBASE_H
#define __CONTROLLERBASE_H

#include "abstractcontroller.h"
#include "matrix.h"
#include "noisegenerator.h"
#include "circular_buffer.h"
#include <memory>

namespace lpzrobots {

/**
 * @brief Base class for controllers with common initialization patterns
 * 
 * This class extracts common initialization code found in many controllers,
 * reducing code duplication while maintaining flexibility.
 * 
 * Features:
 * - Standard model matrices (A, C) initialization
 * - Extended model matrix (S) support
 * - Bias vectors (h, b) initialization
 * - Buffer management
 * - Noise generator support
 * - Teaching infrastructure
 * 
 * @example
 * ```cpp
 * class MyController : public ControllerBase {
 * public:
 *     MyController() : ControllerBase("MyController", "$Id$") {}
 *     
 *     void init(int sensornumber, int motornumber, RandGen* randGen) override {
 *         ControllerBase::init(sensornumber, motornumber, randGen);
 *         
 *         // Initialize standard matrices with identity
 *         initModelMatrices(1.0);  // C *= 1.0
 *         
 *         // Initialize bias vectors
 *         initBiasVectors();
 *         
 *         // Controller-specific initialization...
 *     }
 * };
 * ```
 */
class ControllerBase : public AbstractController {
public:
    /// Constructor
    ControllerBase(const std::string& name, const std::string& revision)
        : AbstractController(name, revision)
        , number_sensors(0)
        , number_motors(0)
        , t(0) {}
    
    /// Destructor
    virtual ~ControllerBase() = default;
    
    /// Basic initialization - must be called by derived classes
    virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override {
        number_sensors = sensornumber;
        number_motors = motornumber;
        t = 0;
        
        // Initialize current sensor and motor vectors
        x.set(number_sensors, 1);
        y.set(number_motors, 1);
    }
    
    /// Returns the number of sensors
    virtual int getSensorNumber() const override {
        return number_sensors;
    }
    
    /// Returns the number of motors
    virtual int getMotorNumber() const override {
        return number_motors;
    }
    
protected:
    // Common member variables
    int number_sensors;    ///< Number of sensors
    int number_motors;     ///< Number of motors
    int t;                ///< Time step counter
    
    // Standard matrices used by most controllers
    matrix::Matrix A;      ///< Model matrix (world model)
    matrix::Matrix C;      ///< Controller matrix
    matrix::Matrix S;      ///< Extended model matrix (sensor-sensor connections)
    matrix::Matrix h;      ///< Controller bias
    matrix::Matrix b;      ///< Model bias
    matrix::Matrix L;      ///< Jacobian matrix
    matrix::Matrix R;      ///< Response matrix
    
    // Current values
    matrix::Matrix x;      ///< Current sensor values
    matrix::Matrix y;      ///< Current motor values
    matrix::Matrix x_smooth; ///< Smoothed sensor values
    
    // Teaching support
    matrix::Matrix y_teaching; ///< Teaching motor signal
    bool intern_isTeaching = false; ///< Is teaching active?
    
    // Noise generators
    std::unique_ptr<NoiseGenerator> BNoiseGen; ///< Noise generator for bias
    std::unique_ptr<NoiseGenerator> YNoiseGen; ///< Noise generator for motor values
    
    /// Initialize standard model matrices A and C with identity
    void initModelMatrices(double cInitFactor = 1.0) {
        A.set(number_sensors, number_motors);
        C.set(number_motors, number_sensors);
        A.toId();
        C.toId();
        C *= cInitFactor;
    }
    
    /// Initialize bias vectors h and b
    void initBiasVectors() {
        h.set(number_motors, 1);
        b.set(number_sensors, 1);
    }
    
    /// Initialize extended model matrix S
    void initExtendedModel(double sFactor = 0.05) {
        S.set(number_sensors, number_sensors);
        S.toId();
        S *= sFactor;
    }
    
    /// Initialize Jacobian and response matrices
    void initJacobianMatrices() {
        L.set(number_sensors, number_sensors);
        R.set(number_sensors, number_sensors);
    }
    
    /// Initialize teaching infrastructure
    void initTeaching() {
        y_teaching.set(number_motors, 1);
        intern_isTeaching = false;
    }
    
    /// Initialize smoothing vector
    void initSmoothing() {
        x_smooth.set(number_sensors, 1);
    }
    
    /// Initialize noise generators
    void initNoiseGenerators(RandGen* randGen) {
        BNoiseGen = std::make_unique<WhiteUniformNoise>();
        BNoiseGen->init(number_sensors, randGen);
        
        YNoiseGen = std::make_unique<WhiteUniformNoise>();
        YNoiseGen->init(number_motors, randGen);
    }
    
    /// Helper function for activation function g(x) = tanh(x)
    static double g(double x) {
        return tanh(x);
    }
    
    /// Helper function for derivative of g: g'(x) = 1 - tanh^2(x)
    static double g_s(double x) {
        double y = tanh(x);
        return 1.0 - y * y;
    }
    
    /// Helper function for clipping values
    static double clip(double x, double limit) {
        return std::max(-limit, std::min(limit, x));
    }
};

/**
 * @brief Extended base class with buffer support
 * 
 * This template class adds circular buffer support for controllers
 * that need to store historical sensor and motor values.
 * 
 * @tparam BufferSize Size of the circular buffers
 */
template<std::size_t BufferSize>
class BufferedControllerBase : public ControllerBase {
public:
    static constexpr std::size_t buffersize = BufferSize;
    
    BufferedControllerBase(const std::string& name, const std::string& revision)
        : ControllerBase(name, revision) {}
    
    virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override {
        ControllerBase::init(sensornumber, motornumber, randGen);
        initBuffers();
    }
    
protected:
    MatrixBuffer<buffersize> x_buffer; ///< Buffer for sensor values
    MatrixBuffer<buffersize> y_buffer; ///< Buffer for motor values
    
    /// Initialize buffers with zero matrices
    void initBuffers() {
        matrix::Matrix x_init(number_sensors, 1);
        matrix::Matrix y_init(number_motors, 1);
        x_buffer.fill(x_init);
        y_buffer.fill(y_init);
    }
    
    /// Get delayed sensor values
    const matrix::Matrix& getDelayedSensors(int delay = 1) const {
        return x_buffer.get(-delay);
    }
    
    /// Get delayed motor values
    const matrix::Matrix& getDelayedMotors(int delay = 1) const {
        return y_buffer.get(-delay);
    }
    
    /// Store current values in buffers
    void updateBuffers(const matrix::Matrix& x_current, const matrix::Matrix& y_current) {
        x_buffer.push(x_current);
        y_buffer.push(y_current);
    }
};

} // namespace lpzrobots

#endif // __CONTROLLERBASE_H