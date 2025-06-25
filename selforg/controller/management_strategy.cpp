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

#include "management_strategy.h"
#include <algorithm>
#include <cmath>

using namespace matrix;

namespace lpzrobots {

DampingManagement::DampingManagement(double damping, int interval)
    : damping(damping), interval(interval) {}

void DampingManagement::manage(Matrix& C, Matrix& A, Matrix& h, Matrix& b, int t) {
    if (damping > 0) {
        C *= (1 - damping);
        A *= (1 - damping);
        h *= (1 - damping);
        b *= (1 - damping);
    }
}

KWTAManagement::KWTAManagement(unsigned int k, double damping, int interval)
    : k(k), damping(damping), interval(interval) {}

void KWTAManagement::manage(Matrix& C, Matrix& A, Matrix& h, Matrix& b, int t) {
    kwtaInhibition(C, k, damping);
}

void KWTAManagement::kwtaInhibition(Matrix& wm, unsigned int k, double damping) {
    const int n = wm.getN();
    const int m = wm.getM();
    
    if (k > static_cast<unsigned int>(n))
        k = n;
    
    for (int i = 0; i < m; i++) {
        // Get absolute values for this row
        std::vector<std::pair<double, int>> abs_values;
        for (int j = 0; j < n; j++) {
            abs_values.push_back({fabs(wm.val(i, j)), j});
        }
        
        // Sort by absolute value (descending)
        std::sort(abs_values.begin(), abs_values.end(), 
                 [](const auto& a, const auto& b) { return a.first > b.first; });
        
        // Set all but the k largest elements to damping * value
        for (int j = k; j < n; j++) {
            wm.val(i, abs_values[j].second) *= damping;
        }
    }
}

ReceptiveFieldManagement::ReceptiveFieldManagement(unsigned int rfSize, int interval)
    : rfSize(rfSize), interval(interval) {}

void ReceptiveFieldManagement::manage(Matrix& C, Matrix& A, Matrix& h, Matrix& b, int t) {
    limitC(C, rfSize);
}

void ReceptiveFieldManagement::limitC(Matrix& wm, unsigned int rfSize) {
    const int n = wm.getN();
    const int m = wm.getM();
    const double minVal = 0.001;
    
    if (rfSize > static_cast<unsigned int>(n))
        return;
    
    int distance = (n - rfSize) / 2;
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (abs(i - j) > distance) {
                if (fabs(wm.val(i, j)) < minVal)
                    wm.val(i, j) = 0;
            }
        }
    }
}

} // namespace lpzrobots