/***************************************************************************
 *   Copyright (C) 2005-2011 by                                            *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_2__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/
#ifndef __INVERTNCHANNELCONTROLLER_H
#define __INVERTNCHANNELCONTROLLER_H

#include "controller_misc.h"
#include "invertcontroller.h"

#include <cassert>
#include <cmath>

#include <selforg/matrix.h>

/**
 * class for N-channel invert controller
 */
class InvertNChannelController : public InvertController {

public:
  InvertNChannelController(int _buffersize, bool _update_only_1 = false);
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) override;

  virtual ~InvertNChannelController() override;

  /// returns the name of the object (with version number)
  virtual paramkey getName() const noexcept override {
    return name;
  }
  /// returns the number of sensors the controller was initialised with or 0 if not initialised
  virtual int getSensorNumber() const override {
    return number_channels;
  }
  /// returns the mumber of motors the controller was initialised with or 0 if not initialised
  virtual int getMotorNumber() const override {
    return number_channels;
  }

  /// performs one step (includes learning).
  /// Calulates motor commands from sensor inputs.
  virtual void step(const sensor*, int number_sensors, motor*, int number_motors) override;

  /// performs one step without learning. Calulates motor commands from sensor inputs.
  virtual void stepNoLearning(const sensor*,
                              int number_sensors,
                              motor*,
                              int number_motors) override;

  /***** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const;
  /** loads the controller values from a given file. */
  virtual bool restore(FILE* f);

  // inspectable interface
  virtual std::list<ILayer> getStructuralLayers() const override;
  virtual std::list<IConnection> getStructuralConnections() const override;

protected:
  unsigned short number_channels = 0;
  unsigned short buffersize = 0;
  bool update_only_1 = false;

  matrix::Matrix A; // Model Matrix
  matrix::Matrix C; // Controller Matrix
  matrix::Matrix h; // Controller Bias
  matrix::Matrix L; // Jacobi Matrix
  matrix::Matrix* x_buffer;
  matrix::Matrix* y_buffer;
  int t = 0;
  paramkey name;

  /*   virtual void iteration(double *column, */
  /*                          double dommy[NUMBER_CHANNELS][NUMBER_CHANNELS], */
  /*                          double *improvment); */

  virtual double calculateE(const matrix::Matrix& x_delay, const matrix::Matrix& y_delay);

  /// learn values h,C
  virtual void learn(const matrix::Matrix& x_delay, const matrix::Matrix& y_delay);

  virtual void learnmodel(const matrix::Matrix& y_delay);

  /// calculate delayed values
  virtual matrix::Matrix calculateDelayedValues(const matrix::Matrix* buffer,
                                                unsigned int number_steps_of_delay_);
  virtual matrix::Matrix calculateSmoothValues(const matrix::Matrix* buffer,
                                               unsigned int number_steps_for_averaging_);

  matrix::Matrix calculateControllerValues(const matrix::Matrix& x_smooth);

  // put new value in ring buffer
  void putInBuffer(matrix::Matrix* buffer, const matrix::Matrix& vec);

  /// neuron transfer function
  static double g(double z) {
    return tanh(z);
  };

  ///
  static double g_s(double z) {
    double k = tanh(z);
    return 1.0 - k * k;
    //    return 1.0 - tanh(z)*tanh(z);
  };

  /// squashing function, to protect against to large weight updates
  static double squash(double z) {
    return clip(z, -0.1, 0.1);
    //    return z < -0.1 ? -0.1 : ( z > 0.1 ? 0.1 : z );
    // return 0.1 * tanh(10.0 * z);
  };
};

#endif
