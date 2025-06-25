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
#ifndef __MANAGEMENT_STRATEGY_H
#define __MANAGEMENT_STRATEGY_H

#include "matrix.h"
#include <vector>

namespace lpzrobots {

/**
 * @brief Abstract interface for controller management strategies
 * 
 * Management strategies handle periodic maintenance tasks like
 * - Parameter decay/damping
 * - K-winner-take-all inhibition
 * - Connection pruning
 * - Matrix normalization
 */
class ManagementStrategy {
public:
    virtual ~ManagementStrategy() = default;
    
    /**
     * @brief Perform management operations on controller matrices
     * 
     * @param C Controller matrix (will be modified)
     * @param A Model matrix (will be modified)
     * @param h Controller bias (will be modified)
     * @param b Model bias (will be modified)
     * @param t Current time step
     */
    virtual void manage(matrix::Matrix& C,
                       matrix::Matrix& A,
                       matrix::Matrix& h,
                       matrix::Matrix& b,
                       int t) = 0;
    
    /**
     * @brief Check if management should run at this timestep
     */
    virtual bool shouldManage(int t) const = 0;
};

/**
 * @brief Basic management with damping
 * 
 * Applies exponential decay to parameters
 */
class DampingManagement : public ManagementStrategy {
public:
    explicit DampingManagement(double damping = 0.00001, int interval = 10);
    
    void manage(matrix::Matrix& C,
               matrix::Matrix& A,
               matrix::Matrix& h,
               matrix::Matrix& b,
               int t) override;
    
    bool shouldManage(int t) const override {
        return (t % interval) == 0;
    }
    
private:
    double damping;
    int interval;
};

/**
 * @brief K-Winner-Take-All inhibition management
 * 
 * Implements lateral inhibition by keeping only k largest connections
 */
class KWTAManagement : public ManagementStrategy {
public:
    explicit KWTAManagement(unsigned int k, double damping = 0.001, int interval = 100);
    
    void manage(matrix::Matrix& C,
               matrix::Matrix& A,
               matrix::Matrix& h,
               matrix::Matrix& b,
               int t) override;
    
    bool shouldManage(int t) const override {
        return (t % interval) == 0;
    }
    
private:
    unsigned int k;
    double damping;
    int interval;
    
    void kwtaInhibition(matrix::Matrix& wm, unsigned int k, double damping);
};

/**
 * @brief Limit receptive field size management
 * 
 * Constrains the number of connections per motor neuron
 */
class ReceptiveFieldManagement : public ManagementStrategy {
public:
    explicit ReceptiveFieldManagement(unsigned int rfSize, int interval = 100);
    
    void manage(matrix::Matrix& C,
               matrix::Matrix& A,
               matrix::Matrix& h,
               matrix::Matrix& b,
               int t) override;
    
    bool shouldManage(int t) const override {
        return (t % interval) == 0;
    }
    
private:
    unsigned int rfSize;
    int interval;
    
    void limitC(matrix::Matrix& wm, unsigned int rfSize);
};

/**
 * @brief Composite management strategy
 * 
 * Combines multiple management strategies
 */
class CompositeManagement : public ManagementStrategy {
public:
    void addStrategy(std::unique_ptr<ManagementStrategy> strategy) {
        strategies.push_back(std::move(strategy));
    }
    
    void manage(matrix::Matrix& C,
               matrix::Matrix& A,
               matrix::Matrix& h,
               matrix::Matrix& b,
               int t) override {
        for (auto& strategy : strategies) {
            if (strategy->shouldManage(t)) {
                strategy->manage(C, A, h, b, t);
            }
        }
    }
    
    bool shouldManage(int t) const override {
        for (const auto& strategy : strategies) {
            if (strategy->shouldManage(t)) {
                return true;
            }
        }
        return false;
    }
    
private:
    std::vector<std::unique_ptr<ManagementStrategy>> strategies;
};

} // namespace lpzrobots

#endif // __MANAGEMENT_STRATEGY_H