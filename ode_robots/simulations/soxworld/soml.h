/***************************************************************************
 *   Copyright (C) 2010 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    der@informatik.uni-leipzig.de                                        *
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
 *                                            *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/
#ifndef __SOML_H
#define __SOML_H

#include <selforg/abstractcontroller.h>
#include <selforg/controller_misc.h>
#include <selforg/controllernet.h>

#include <cassert>
#include <cmath>

#include <selforg/matrix.h>

/// Configuration object for SoML controller
struct SoMLConf {
  bool useHiddenContr = false; ///< use a hidden layer in the controller network?
  /// ratio of motor units and hidden units in the controller (2 -> double amount of hidden unit)
  double hiddenContrUnitsRatio = 0;
  bool useHiddenModel = false; ///< use a hiddenlayer in the model network?
  /// ratio of motor units and hidden units in the model (2 -> double amount of hidden unit)
  double hiddenModelUnitsRatio = 0;
  bool useS = false;         ///< direct connection from x_t to xp_t+1
  bool initUnitMatrix = false; /// if true then the network is initialized with unit matrices

  bool someInternalParams = false; ///< only export some internal parameters
};

/**
 * This controller implements the homeokinetic learning algorihm
 * in sensor space with extended controller network
 */
class SoML{

public:
  SoML(const SoMLConf& conf = getDefaultConf());

  static SoMLConf getDefaultConf() const {
    SoMLConf c;
    c.useHiddenContr = true;
    c.useHiddenModel = true;
    c.hiddenContrUnitsRatio = 1.0;
    c.hiddenModelUnitsRatio = 1.0;
    c.someInternalParams=false;
    c.useS = false;
    c.initUnitMatrix = true;
    return c;
  }

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0);

  virtual ~SoML() override;

  /// returns the number of sensors the controller was initialised with or 0 if not initialised
  virtual int getSensorNumber() const { return number_sensors; }
  /// returns the mumber of motors the controller was initialised with or 0 if not initialised
  virtual int getMotorNumber() const { return number_motors; }

  /// performs one step (includes learning).
  /// Calulates motor commands from sensor inputs.
  virtual void step(const sensor* , int number_sensors, motor* , int number_motors);

  /// performs one step without learning. Calulates motor commands from sensor inputs.
  virtual void stepNoLearning(const sensor* , int number_sensors,
                              motor* , int number_motors);

  // motor babbling: learn the basic relations from observed sensors/motors
  virtual void motorBabblingStep(const sensor* , int number_sensors,
                                 const motor* , int number_motors);


  /***** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const override;
  /** loads the controller values from a given file. */
  virtual bool explicit restore(FILE* f);

  /// returns controller network (to be added to inspectables of agent)
  virtual ControllerNet* getCNet() const;

protected:
  /// performs control step (activates network and stores results in buffer and y_)
  void control(const matrix::Matrix& x, motor* y_, int number_motors);

  /** learn values model and controller network
      using the current sensors x, the commands y (from last step (or earlier in case of delay))
   */
  virtual void learn(const matrix::Matrix& x, const matrix::Matrix& y);

  /* learns the model using backprop. It uses the current activation,
     the current x and x_tm1 from the buffer */
  virtual void explicit learnModelBP(double factor);

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;
  static const unsigned short buffersize = 10;

  matrix::Matrix y_buffer[buffersize]; ///< buffer needed for delay
  matrix::Matrix x_buffer[buffersize]; ///< buffer needed for delay
  ControllerNet* cNet; ///< Controller network
  unsigned int numControllerLayer = 0; ///< number of controller layer

  SoMLConf conf; ///< configuration object

  matrix::Matrix x;        // current sensor value vector
  matrix::Matrix x_smooth; // time average of x values
  matrix::Matrix eta_avg;    // time average of shift (in motor space)
  int t = 0;
  double E = 0;

  paramval creativity;
  paramval epsC;
  paramval epsA;
  paramval harmony;        ///< harmony
  paramval dampA;
  paramval discountS;      ///< discount for S part of the model
  paramint s4avg;          ///< # of steps the sensors are averaged (1 means no averaging)
  paramint s4delay;        ///< # of steps the motor values are delayed (1 means no delay)
  paramval biasnoise;
  //  parambool logaE;         ///< # use logarithmic error



};

#endif


