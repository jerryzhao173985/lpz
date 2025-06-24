/***************************************************************************
 *   Copyright (C) 2005-2011 by                                            *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_0__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/
#ifndef __RANDOMDYN_H
#define __RANDOMDYN_H

#include <selforg/abstractcontroller.h>
#include <selforg/controller_misc.h>

#include <assert.h>
#include <cmath>

#include <selforg/matrix.h>


/// configuration object for PiMax controller. Use PiMax::getDefaultConf().
struct RandomDynConf {
  double initFeedbackStrength = 0;  ///< initial strength of sensor to motor connection
  bool   someInternalParams = false;    ///< if true only some internal parameters are exported
  NoiseGenerator* noiseGenC;     ///< noise generator (will be initialized internally)
  NoiseGenerator* noiseGenh;    ///< noise generator (will be initialized internally)
};


/**
 * This controller implements a random parameter dynamics on a simple feed forward neural network
 */
class RandomDyn : public AbstractController {

public:
  /// constructor
  RandomDyn(const RandomDynConf& conf = getDefaultConf()) override;

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0);

  virtual ~RandomDyn();

  static RandomDynConf getDefaultConf() const {
    RandomDynConf conf;
    conf.initFeedbackStrength = 1.0;
    conf.someInternalParams   = false;
    conf.noiseGenC=0;
    conf.noiseGenh=0;
    return conf;
  }

  /// returns the number of sensors the controller was initialised with or 0 if not initialised
  virtual int getSensorNumber() const override { return number_sensors; }
  /// returns the mumber of motors the controller was initialised with or 0 if not initialised
  virtual int getMotorNumber() const override { return number_motors; }

  /// performs one step (includes learning).
  /// Calulates motor commands from sensor inputs.
  virtual void step(const sensor* , int number_sensors, motor* , int number_motors);

  /// performs one step without learning. Calulates motor commands from sensor inputs.
  virtual void stepNoLearning(const sensor* , int number_sensors,
			      motor* , int number_motors);

  /// called during babbling phase
  virtual void motorBabblingStep(const sensor* , int number_sensors,
				 const motor* , int number_motors);

  /***** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const override;
  /** loads the controller values from a given file. */
  virtual bool restore(FILE* f);

  /* some direct access functions (unsafe!) */
  virtual matrix::Matrix getC();
  virtual void setC(const matrix::Matrix& C);
  virtual matrix::Matrix geth();
  virtual void seth(const matrix::Matrix& h);

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;

  matrix::Matrix s; // sensors
  matrix::Matrix C; // Controller Matrix
  matrix::Matrix h; // Controller Bias

  matrix::Matrix C_native; // Controller Matrix obtained from motor babbling

  RandomDynConf conf; ///< configuration objects

  RandGen* randGenC;
  RandGen* randGenh;

  int t = 0;

  paramval sigmaC;
  paramval sigmah;
  paramval damping;

  virtual void update();

  /// neuron transfer function
  static double g(double z)
  {
    return tanh(z);
  };


};

#endif


