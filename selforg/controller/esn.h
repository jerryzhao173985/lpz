/*****************************************************************************
 *             FIAS winter school, playful machine group                     *
 *                Supervisors: Ralf Der, Georg Martius                       *

 * Members: Fabien Benureau, Chrisantha Fernando, Quan Wang, Jimmy Baraglia  *
 *                   Echo State Network Header File                          *
 *                                                                           *
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
#ifndef __ESN_H
#define __ESN_H

#include <cmath>
#include <selforg/invertablemodel.h>
#include <selforg/matrix.h>
#include <cstdio>

struct ESNConf {
  int numNeurons = 100;
  double inputStrength = 0.1;
  double inputRatio = 1.0;
  double connectionRatio = 0.1;
  double spectralRadius = 0.9;
  bool inspectInternals = false;
  double learningrate = 0.01;
};

/**
 * class for Echo State Networks (ESN)
 */
class ESN : public InvertableModel {
public:
  /**
     @param controlmask bitmask to select channels to control (default all)
     @param function controller function to use
   */
  ESN(const ESNConf& conf = getDefaultConf());

  static ESNConf getDefaultConf() {
    ESNConf c;
    c.numNeurons = 100;
    c.inputStrength = 0.1;
    c.inputRatio = 1;
    c.connectionRatio = 0.1;
    c.spectralRadius = 0.9;
    c.inspectInternals = false;
    c.learningrate = 0.01;
    return c;
  }

  /** initialisation of the network with the given number of input and output units
      @param inputDim length of input vector
      @param outputDim length of output vector
      @param unit_map if 0 the parametes are choosen randomly.
             Otherwise the model is initialised to represent a unit_map
             with the given response strength.
      @param randGen pointer to random generator, if 0 an new one is used
   */
  virtual void init(unsigned int inputDim,
                    unsigned int outputDim,
                    double unit_map = 0.0,
                    RandGen* randGen = nullptr) override;

  /** passive processing of the input
     (this function is not constant since a recurrent network
     for example might change internal states
  */
  virtual const matrix::Matrix process(const matrix::Matrix& input) override;

  /* performs learning and returns the network output before learning.
     Neural networks process the input before. (no need to call process before)
     \param learnRateFactor can be given to modify eps for this learning step.
  */
  virtual const matrix::Matrix learn(const matrix::Matrix& input,
                                     const matrix::Matrix& nom_output,
                                     double learnRateFactor = 1) override;

  /* calculates the partial derivative of the of the output with repect to the input(const Jacobi& matrix).

      \f[J_{ij} = \frac{\partial output_i}{\partial input_j}\f]

      The input is ignored, the network must  be processed or learned before!
   */
  virtual const matrix::Matrix response(const matrix::Matrix& _input) const override;

  /* calculates the input shift v to given output shift xsi via pseudo inversion.

      \f[o+\xi = \pi(i+v)\f]

      The input is ignored, the network must  be processed or learned before!
   */
  virtual const matrix::Matrix inversion(const matrix::Matrix& input,
                                         const matrix::Matrix& xsi) const override;

  /// damps the weights and the biases by multiplying (1-damping)
  virtual void damp(double damping) override;

  /// returns the number of input neurons
  virtual unsigned int getInputDim() const override;
  /// returns the number of output neurons
  virtual unsigned int getOutputDim() const override;

  virtual bool store(FILE* f) const override;

  virtual bool restore(FILE* f) override;

  static double tanh_prime(double z) {
    double k = tanh(z);
    return 1.0 - k * k;
  };

protected:
  ESNConf conf;

  int nbInputs;
  int nbOutputs;
  matrix::Matrix inputWeights;
  matrix::Matrix outputWeights;
  matrix::Matrix outputDirectWeights;
  matrix::Matrix ESNState;
  matrix::Matrix ESNActivations;
  matrix::Matrix ESNWeights;
  double error;
  bool initialized;

  //
};

#endif
