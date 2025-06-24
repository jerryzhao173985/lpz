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
#ifndef __NOISEGENERATOR_H
#define __NOISEGENERATOR_H

#include <assert.h>
#include <cmath>
#include <stdlib.h>
#include <time.h>

#include "randomgenerator.h"

/** Interface and basic class for noise generator.
    It is suitable for single noise channels but also multidimensional noise.
 */
class NoiseGenerator {
public:
  NoiseGenerator() {
    dimension = 0;
    randGen = 0;
    ownRandGen = false;
  };

  virtual ~NoiseGenerator() {
    if (this->ownRandGen && this->randGen) {
      delete this->randGen;
      this->randGen = 0;
      this->ownRandGen = false;
    }
  }

  /** initialization with the the given dimension for multidimensional noise
      @param dimension dimensionality of vectors to be used by add
      @param randGen pointer to a random generator. If zero a new one generated internally
      @see add()
   */
  virtual void init(unsigned int dimension, RandGen* randGen = 0) {
    this->dimension = dimension;
    if (randGen)
      this->randGen = randGen;

    else {
      this->randGen = new RandGen();
      this->randGen->init(rand());
      this->ownRandGen = true;
    }
  };

  /** generate somehow distributed random number parameterized with min and max.
      valid only for ONE random number, use \ref add() for
      adding this kind of noise to several channels
   */
  virtual double generate() = 0;

  /** adds multidimensional noise to the value field.
      Generic implementation calls generate for each channel.
      Overload this if you need different behavior.
      @param value field where noise is added. Must have length dimension (\ref init())
   */
  virtual void add(double* value, double noiseStrength) {
    for (unsigned int i = 0; i < dimension; ++i) {
      value[i] += generate() * noiseStrength;
    }
  }

  virtual unsigned int getDimension() const {
    return dimension;
  }

  virtual void setDimension(unsigned int dim) {
    if (dimension != dim) {
      init(dim, randGen);
    }
  }

protected:
  // generates white (no averaging) uniformly distributed random number between __PLACEHOLDER_1__ and __PLACEHOLDER_2__
  double uniform(double min = -0.1, double max = 0.1) {
    assert(randGen);
    return (randGen->rand() * (max - min) + min);
  }
  // generates white uniformly distributed random number between in [0,1)
  double uniform01() {
    assert(randGen);
    return randGen->rand();
  }
  unsigned int dimension;
  RandGen* randGen;
  bool ownRandGen;
};

/// generates no noise
class NoNoise : public NoiseGenerator {
public:
  NoNoise() {}
  virtual ~NoNoise() {}
  virtual double generate() override {
    return 0;
  };
};

/// generates white (no averaging) uniformly distributed random number between __PLACEHOLDER_3__ and __PLACEHOLDER_4__
class WhiteUniformNoise : public NoiseGenerator {
public:
  WhiteUniformNoise() {}
  virtual ~WhiteUniformNoise() {}
  virtual double generate() override {
    return uniform(-1, 1);
  };
};

/// generates white and normal distributed random numbers. p1: mean, p2: standard deviation
/// new parameter definition: p1: min, p2: max. the mean and standard deviation are calculated by
/// this values
class WhiteNormalNoise : public NoiseGenerator {
public:
  WhiteNormalNoise() {}
  virtual ~WhiteNormalNoise() {}
  virtual double generate() override {
    double x1 = uniform01();
    double x2 = uniform01();
    return ((sqrt(-2 * log(x1)) * cos(2 * M_PI * x2)));
  };
  // original version
  //  virtual double generate(double mean, double stddev) {
  //    double x1=uniform(0, 1);
  //    double x2=uniform(0, 1);
  //    return( (sqrt(-2*log(x1)) *cos(2*M_PI*x2))  * stddev + mean) ;
  //  }
};

/// generated colored noise. This is obtained by using time average of uniform distributed noise.
class ColorUniformNoise : public NoiseGenerator {
public:
  /** @param tau time averaging factor (1/window)
      (1: smoothing (white) 0.1: strong color, 0 no noise anymore
  */
  ColorUniformNoise(double tau = 0.05)
    : tau(tau) {
    sqrttau = sqrt(tau);
    mean1channel = 0.0;
    mean = 0;
  }
  virtual ~ColorUniformNoise() {
    if (mean)
      free(mean);
  }
  virtual void init(unsigned int dimension, RandGen* randGen = 0) override {
    NoiseGenerator::init(dimension, randGen);
    mean = static_cast<double*>(malloc(sizeof(double) * dimension));
    if (!mean) {
      std::cerr << "ColorUniformNoise: Memory allocation failed\n";
      exit(1);
    }
    for (unsigned int i = 0; i < dimension; ++i) {
      mean[i] = 0.0;
    }
  }

  virtual double generate() override {
    mean1channel += sqrttau * uniform(-1, +1) - tau * mean1channel;
    return (mean1channel);
  }

  /** adds multidimensional noise to the value field.
      @param value field where noise is added. Must have length dimension (\ref init())
      @param min lower bound of interval
      @param max upper bound of interval
   */
  virtual void add(double* value, double noiseStrength) override {
    for (unsigned int i = 0; i < dimension; ++i) {
      mean[i] += sqrttau * uniform(-1, +1) * noiseStrength - tau * mean[i];
      value[i] += mean[i];
    }
  }

  virtual double getTau() {
    return tau;
  }
  virtual void setTau(double newTau) {
    if (newTau >= 0 && newTau <= 1) {
      tau = newTau;
      sqrttau = sqrt(tau);
    }
  }

protected:
  double tau = 0;     // smoothing paramter
  double sqrttau = 0; // square root of tau
  double mean1channel = 0; // mean for single channel
  double* mean;
};

/// like ColorUniformNoise but averaging over normal distributed noise instead.
class ColorNormalNoise : public WhiteNormalNoise {
public:
  ColorNormalNoise(double tau = 0.05)
    : tau(tau)
    , sqrttau(sqrt(tau))
    , mean(0)
    , mean1channel(0.0)
    , factor(0.0) {}

  virtual ~ColorNormalNoise() {
    if (mean)
      free(mean);
  }

  virtual void init(unsigned int dimension, RandGen* randGen = 0) override {
    WhiteNormalNoise::init(dimension, randGen);
    mean = static_cast<double*>(malloc(sizeof(double) * dimension));
    if (!mean) {
      std::cerr << "ColorNormalNoise: Memory allocation failed\n";
      exit(1);
    }
    for (unsigned int i = 0; i < dimension; ++i) {
      mean[i] = 0.0;
    }
  }

  virtual double generate() override { // double stddev, double meanvalue) {
    mean1channel += sqrttau * WhiteNormalNoise::generate() - tau * mean1channel;
    return (mean1channel);
  }

  virtual void add(double* value, double noiseStrength) override {
    for (unsigned int i = 0; i < dimension; ++i) {
      mean[i] += sqrttau * WhiteNormalNoise::generate() * noiseStrength - tau * mean[i];
      value[i] += mean[i];
    }
  }

  virtual double getTau() {
    return tau;
  }
  virtual void setTau(double newTau) {
    if (newTau >= 0 && newTau <= 1) {
      tau = newTau;
      sqrttau = sqrt(tau);
    }
  }

protected:
  double tau = 0;     // smoothing paramter
  double sqrttau = 0; // square root of smoothing parameter
  double* mean;
  double mean1channel = 0;
  double factor = 0;
};

/// Sine wave noise. Produces a 90 degree phase shifted sine wave or white noise
class SineWhiteNoise : public NoiseGenerator {
public:
  /** @param omega anglerate
      @param amplitude weighting of sine wave against noise strength
      @param phaseShift phase shift between channels in rad
      @param channels number of channel for sine noise (and the rest get white noise)
   */
  SineWhiteNoise(double omega,
                 double amplitude,
                 double phaseShift = M_PI / 2,
                 unsigned int channels = 0xFFFF)
    : omega(omega)
    , amplitude(amplitude)
    , channels(channels)
    , phaseShift(phaseShift) {
    t = 0;
  }

  virtual ~SineWhiteNoise() {}

  virtual double generate() override {
    ++t;
    return (1 - amplitude) * uniform(-1, 1) + sin(t * omega) * amplitude;
  }

  /** adds multidimensional noise to the value field.
      @param value field where noise is added. Must have length dimension (\ref init())
   */
  virtual void add(double* value, double noiseStrength) override { // min, double max){

    for (unsigned int i = 0; i < dimension; ++i) {
      if (i < channels) {
        value[i] += sin(t * omega + i * phaseShift) * amplitude * noiseStrength;
        value[i] += (1 - amplitude) * uniform(-1, 1) * noiseStrength;
      } else {
        value[i] += uniform(-1, 1) * noiseStrength;
      }
    }
    ++t;
  }
  void setOmega(double omega) {
    this->omega = omega;
  }
  void setPhaseShift(double phaseShift) {
    this->phaseShift = phaseShift;
  }

protected:
  long int t;            // time
  double omega = 0;          // angle velocity
  double amplitude = 0;      // factor for noise strength
  unsigned int channels = 0; // number of channels with sine
  double phaseShift = 0;     // phase shift
};

#endif
