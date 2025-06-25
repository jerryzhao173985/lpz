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

#include "learning_strategy.h"
#include <selforg/controller_misc.h>

using namespace matrix;

namespace lpzrobots {

HomeokineticsLearning::HomeokineticsLearning(double epsC, double epsA)
    : epsC(epsC), epsA(epsA) {}

std::pair<Matrix, Matrix> 
HomeokineticsLearning::calculateControllerUpdate(const Matrix& C,
                                                 const Matrix& h,
                                                 const Matrix& xsi,
                                                 const Matrix* x_buffer,
                                                 const Matrix* y_buffer,
                                                 int t) {
    // This is a simplified version of the homeokinetic learning rule
    // The full implementation would include all the details from InvertMotorNStep
    
    Matrix C_update(C.getM(), C.getN());
    Matrix h_update(h.getM(), h.getN());
    
    // Get current and past values
    const Matrix& x = x_buffer[t % 50];  // Assuming buffersize 50
    // const Matrix& y = y_buffer[(t-1) % 50];  // Not used in simplified version
    
    // Calculate z (pre-activation)
    const Matrix& z = C * x + h;
    const Matrix& g_prime = z.map(g_s);
    
    // Simple learning rule (simplified from full homeokinetics)
    const Matrix& eta = xsi;  // Simplified - should use pseudoinverse of A
    
    // Controller update
    C_update = (eta * (x^T)) * epsC;
    h_update = eta * epsC;
    
    return std::make_pair(C_update, h_update);
}

std::pair<Matrix, Matrix>
HomeokineticsLearning::calculateModelUpdate(const Matrix& A,
                                           const Matrix& b,
                                           const Matrix& xsi,
                                           const Matrix* x_buffer,
                                           const Matrix* y_buffer,
                                           int t) {
    Matrix A_update(A.getM(), A.getN());
    Matrix b_update(b.getM(), b.getN());
    
    // Get motor values
    const Matrix& y = y_buffer[(t-1) % 50];
    
    // Simple delta rule for model learning
    A_update = (xsi * (y^T)) * epsA;
    b_update = xsi * epsA * 0.1;  // Slower learning for bias
    
    return std::make_pair(A_update, b_update);
}

TeachableHomeokineticsLearning::TeachableHomeokineticsLearning(double epsC, double epsA, double gamma_teach)
    : HomeokineticsLearning(epsC, epsA), gamma_teach(gamma_teach) {}

std::pair<Matrix, Matrix> 
TeachableHomeokineticsLearning::calculateControllerUpdate(const Matrix& C,
                                                          const Matrix& h,
                                                          const Matrix& xsi,
                                                          const Matrix* x_buffer,
                                                          const Matrix* y_buffer,
                                                          int t) {
    // First get the basic homeokinetic update
    auto [C_update, h_update] = HomeokineticsLearning::calculateControllerUpdate(C, h, xsi, x_buffer, y_buffer, t);
    
    // Add teaching component if available
    if (hasTeaching) {
        const Matrix& x = x_buffer[t % 50];
        const Matrix& y = y_buffer[(t-1) % 50];
        
        // Teaching error
        const Matrix& teaching_error = y_teaching - y;
        
        // Add teaching update
        C_update += (teaching_error * (x^T)) * (gamma_teach * epsC);
        h_update += teaching_error * (gamma_teach * epsC);
        
        // Clear teaching signal after use
        hasTeaching = false;
    }
    
    return std::make_pair(C_update, h_update);
}

} // namespace lpzrobots