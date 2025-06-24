/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
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
#ifndef __MODERN_RANDOMGENERATOR_H
#define __MODERN_RANDOMGENERATOR_H

#include <chrono>
#include <random>

/// Modern C++17 random generator using Mersenne Twister
class ModernRandGen {
public:
  ModernRandGen()
    : gen(std::chrono::steady_clock::now().time_since_epoch().count())
    , dist(0.0, 1.0) {}

  explicit ModernRandGen(long int seedval)
    : gen(seedval)
    , dist(0.0, 1.0) {}

  void init(long int seedval) {
    gen.seed(seedval);
  }

  /// returns a value in [0,1)
  double rand() {
    return dist(gen);
  }

  /// returns a value in [min, max)
  double rand(double min, double max) {
    std::uniform_real_distribution<double> local_dist(min, max);
    return local_dist(gen);
  }

  /// returns an integer value in [min, max]
  int randInt(int min, int max) {
    std::uniform_int_distribution<int> int_dist(min, max);
    return int_dist(gen);
  }

  /// returns a normally distributed value with given mean and stddev
  double randGauss(double mean = 0.0, double stddev = 1.0) {
    std::normal_distribution<double> gauss_dist(mean, stddev);
    return gauss_dist(gen);
  }

private:
  std::mt19937 gen;                            // Mersenne Twister generator
  std::uniform_real_distribution<double> dist; // Distribution for [0,1)
};

// Legacy compatibility typedef
using RandGen = ModernRandGen;

#endif