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
#ifndef __LEARNING_STRATEGY_H
#define __LEARNING_STRATEGY_H

#include "matrix.h"
#include <memory>

namespace lpzrobots {

/**
 * @brief Abstract interface for learning strategies
 * 
 * This interface defines the contract for different learning algorithms
 * that can be used by controllers. It separates the learning logic from
 * the controller implementation, following the Strategy pattern.
 */
class LearningStrategy {
public:
    virtual ~LearningStrategy() = default;
    
    /**
     * @brief Update controller parameters based on prediction error
     * 
     * @param C Current controller matrix
     * @param h Current controller bias
     * @param xsi Prediction error
     * @param x_buffer Sensor value buffer
     * @param y_buffer Motor value buffer
     * @param t Current time step
     * @return Pair of update matrices (C_update, h_update)
     */
    virtual std::pair<matrix::Matrix, matrix::Matrix> 
    calculateControllerUpdate(const matrix::Matrix& C,
                             const matrix::Matrix& h,
                             const matrix::Matrix& xsi,
                             const matrix::Matrix* x_buffer,
                             const matrix::Matrix* y_buffer,
                             int t) = 0;
    
    /**
     * @brief Update model parameters based on prediction error
     * 
     * @param A Current model matrix
     * @param b Current model bias
     * @param xsi Prediction error
     * @param x_buffer Sensor value buffer
     * @param y_buffer Motor value buffer
     * @param t Current time step
     * @return Pair of update matrices (A_update, b_update)
     */
    virtual std::pair<matrix::Matrix, matrix::Matrix>
    calculateModelUpdate(const matrix::Matrix& A,
                        const matrix::Matrix& b,
                        const matrix::Matrix& xsi,
                        const matrix::Matrix* x_buffer,
                        const matrix::Matrix* y_buffer,
                        int t) = 0;
    
    /**
     * @brief Set learning parameters
     */
    virtual void setLearningRates(double epsC, double epsA) = 0;
};

/**
 * @brief Standard homeokinetic learning strategy
 * 
 * Implements the basic homeokinetic learning algorithm as described
 * in Der & Martius 2011: The Playful Machine
 */
class HomeokineticsLearning : public LearningStrategy {
public:
    explicit HomeokineticsLearning(double epsC = 0.1, double epsA = 0.1);
    
    std::pair<matrix::Matrix, matrix::Matrix> 
    calculateControllerUpdate(const matrix::Matrix& C,
                             const matrix::Matrix& h,
                             const matrix::Matrix& xsi,
                             const matrix::Matrix* x_buffer,
                             const matrix::Matrix* y_buffer,
                             int t) override;
    
    std::pair<matrix::Matrix, matrix::Matrix>
    calculateModelUpdate(const matrix::Matrix& A,
                        const matrix::Matrix& b,
                        const matrix::Matrix& xsi,
                        const matrix::Matrix* x_buffer,
                        const matrix::Matrix* y_buffer,
                        int t) override;
    
    void setLearningRates(double epsC, double epsA) override {
        this->epsC = epsC;
        this->epsA = epsA;
    }
    
protected:
    double epsC;
    double epsA;
    bool logaE = false;
    
    // Helper functions from InvertMotorNStep
    static double g(double z) { return tanh(z); }
    static double g_s(double z) { 
        double k = tanh(z);
        return 1.0 - k * k;
    }
};

/**
 * @brief Extended homeokinetic learning with teaching support
 * 
 * Adds teaching signal support to the basic homeokinetic algorithm
 */
class TeachableHomeokineticsLearning : public HomeokineticsLearning {
public:
    explicit TeachableHomeokineticsLearning(double epsC = 0.1, double epsA = 0.1, double gamma_teach = 0.001);
    
    std::pair<matrix::Matrix, matrix::Matrix> 
    calculateControllerUpdate(const matrix::Matrix& C,
                             const matrix::Matrix& h,
                             const matrix::Matrix& xsi,
                             const matrix::Matrix* x_buffer,
                             const matrix::Matrix* y_buffer,
                             int t) override;
    
    void setTeachingSignal(const matrix::Matrix& teaching) {
        this->y_teaching = teaching;
        hasTeaching = true;
    }
    
    void clearTeachingSignal() {
        hasTeaching = false;
    }
    
private:
    matrix::Matrix y_teaching;
    bool hasTeaching = false;
    double gamma_teach;
};

/**
 * @brief Factory for creating learning strategies
 */
class LearningStrategyFactory {
public:
    enum Type {
        HOMEOKINETICS,
        TEACHABLE_HOMEOKINETICS
    };
    
    static std::unique_ptr<LearningStrategy> create(Type type, double epsC = 0.1, double epsA = 0.1) {
        switch(type) {
            case HOMEOKINETICS:
                return std::make_unique<HomeokineticsLearning>(epsC, epsA);
            case TEACHABLE_HOMEOKINETICS:
                return std::make_unique<TeachableHomeokineticsLearning>(epsC, epsA);
            default:
                return std::make_unique<HomeokineticsLearning>(epsC, epsA);
        }
    }
};

} // namespace lpzrobots

#endif // __LEARNING_STRATEGY_H