/***************************************************************************
 *   Copyright (C) 2010 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_5__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                            *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/
#ifndef __DEP_H
#define __DEP_H

#include "controllerbase.h"
#include <selforg/controller_misc.h>

#include <cassert>
#include <cmath>

#include <selforg/matrix.h>
#include <selforg/ringbuffer.h>

/// configuration object for DEP controller. Use DEP::getDefaultConf().
struct DEPConf {
#define LEARNINGRULES                                                                              \
  X(DEPNormRule, "DEPNormRule")                                                                    \
  X(DEPRule, "DEPRule")                                                                            \
  X(DHLRule, "DHLRule")                                                                            \
  X(HLwFB, "HLwFB")                                                                                \
  X(HLPlain, "HLPlain")

  enum LearningRule {
#define X(Enum, String) Enum,
    LEARNINGRULES
#undef X
  };
  std::map<LearningRule, std::string> LearningRuleNames = {
#define X(Enum, String) { Enum, String },
    LEARNINGRULES
#undef X
  };

  LearningRule learningRule;

  double initFeedbackStrength; ///< initial strength of sensor to motor connection
  bool initModel;              ///< initialize model or leave it 0 to be learned
  bool useExtendedModel;       ///< if true, the extended model (S matrix) is used
  /// # of steps the sensors are averaged (1 means no averaging)

  int steps4Averaging;
  /// # of steps the motor values are delayed (1 means no delay)
  int steps4Delay;
  bool calcEigenvalues; ///< if true calculate the eigenvalues of L

  double factorS; ///< factor for learning rate of S
  double factorh; ///< factor for learning rate of h
};

/**
 * This controller implements a new very much simplified algorihm derived from TiPI maximization
 */
class DEP : public lpzrobots::BufferedControllerBase<150>, public Storeable {

public:
  DEP(const DEPConf& conf = getDefaultConf());
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) override;

  virtual ~DEP() override;

  static DEPConf getDefaultConf() {
    DEPConf conf;
    conf.learningRule = DEPConf::DEPRule;
    conf.initFeedbackStrength = 0;
    conf.useExtendedModel = true;
    conf.steps4Averaging = 1;
    conf.steps4Delay = 1;
    conf.calcEigenvalues = false;
    conf.initModel = true;

    conf.factorS = 0.1;
    conf.factorh = 1;
    return conf;
  }

  /// returns the number of sensors the controller was initialised with or 0 if not initialised
  virtual int getSensorNumber() const override {
    return number_sensors;
  }
  /// returns the mumber of motors the controller was initialised with or 0 if not initialised
  virtual int getMotorNumber() const override {
    return number_motors;
  }

  /// performs one step (includes learning).
  /// Calulates motor commands from sensor inputs.
  virtual void step(const sensor*, int number_sensors, motor*, int number_motors) override;

  /// performs one step without learning. Calulates motor commands from sensor inputs.
  virtual void stepNoLearning(const sensor*, int number_sensors, motor*, int number_motors) override;

  /// called during babbling phase
  virtual void motorBabblingStep(const sensor*,
                                 int number_sensors,
                                 const motor*,
                                 int number_motors) override;

  /***** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const override;
  /** loads the controller values from a given file. */
  virtual bool restore(FILE* f) override;

  // accessors to matrices
  virtual matrix::Matrix getA() {
    return A;
  }
  virtual void setA(const matrix::Matrix& _A) {
    assert(A.getM() == _A.getM() && A.getN() == _A.getN());
    A = _A;
  }

protected:
  // Base class provides: number_sensors, number_motors, buffersize,
  // A, C, S, h, b, L, R, x, y, x_smooth, t
  // x_buffer, y_buffer via BufferedControllerBase
  
  DEPConf conf; // configuration object

  matrix::Matrix C_update; // fast controller matrix (function of immediate history)
  matrix::Matrix normmot;  // factors for individual normalization
  
  // t is inherited from base class

  matrix::Matrix eigenvaluesLRe; // Eigenvalues of L matrix real part
  matrix::Matrix eigenvaluesLIm; // Eigenvalues of L matrix imaginary part
  matrix::Matrix eigenvectors;   // Eigenvectors of L matrix (real part)
  double proj_ev1 = 0;               // projection of x into first eigenvector
  double proj_ev2 = 0;               // projection of x into second eigenvector
  int calcEVInterval = 0;


  paramval epsC;
  paramval epsh;
  paramval epsA;
  paramval norming;
  paramval damping;
  paramint s4avg;   // # of steps the sensors are averaged (1 means no averaging)
  paramint s4delay; // # of steps the motor values are delayed (1 means no delay)

  //  paramval maxSpeed;       ///< maximal speed for motors
  int indnorm = 0;
  int regularization = 0; ///< exponent of regularization 10^{-regularization}

  paramval urate;
  paramval slowfast;
  paramval synboost;

  paramval timedist;
  bool _internWithLearning = false;

  /// learn  model (M = A^T )
  virtual void learnModel(double eps);

  /// learn controller (C,h, C_update)
  virtual void learnController();

  // Note: g(), g_s(), and clip() are inherited from ControllerBase
  
  /// calculates the inverse the argument (useful for Matrix::map)
  static double one_over(double x) {
    return 1 / x;
  }
};

#endif
