/***************************************************************************
 *   Copyright (C) 2005-2011 by                                            *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_1__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/
#ifndef __SOX_H
#define __SOX_H

#include <selforg/abstractcontroller.h>
#include <selforg/controller_misc.h>

#include <cassert>
#include <cmath>

#include <selforg/matrix.h>
#include <selforg/parametrizable.h>
#include <selforg/teachable.h>

/// configuration object for Sox controller. Use Sox::getDefaultConf().
struct SoxConf {
  double initFeedbackStrength = 0; ///< initial strength of sensor to motor connection
  bool useExtendedModel = false;       ///< if true, the extended model (S matrix) is used
  /// if true the controller can be taught see teachable interface
  bool useTeaching = false;
  /// # of steps the sensors are averaged (1 means no averaging)
  int steps4Averaging = 0;
  /// # of steps the motor values are delayed (1 means no delay)
  int steps4Delay = 0;
  bool someInternalParams = false; ///< if true only some internal parameters are exported
  bool onlyMainParameters = false; ///< if true only some configurable parameters are exported

  double factorS = 0; ///< factor for learning rate of S
  double factorb = 0; ///< factor for learning rate of b
  double factorh = 0; ///< factor for learning rate of h
};

/**
 * This controller implements the standard algorihm described the the Chapter 5 (Homeokinesis)
 *  with extensions of Chapter 15 of book Der/Martius 2011: The Playful Machine
 */
class Sox : public AbstractController, public Teachable {

public:
  /// constructor
  Sox(const SoxConf& conf = getDefaultConf());

  /// constructor provided for convenience, use conf object to customize more
  Sox(double init_feedback_strength,
               bool useExtendedModel = true,
               bool useTeaching = false);

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override;

  virtual ~Sox();

  static SoxConf getDefaultConf() {
    SoxConf conf;
    conf.initFeedbackStrength = 1.0;
    conf.useExtendedModel = true;
    conf.useTeaching = false;
    conf.steps4Averaging = 1;
    conf.steps4Delay = 1;
    conf.someInternalParams = false;
    conf.onlyMainParameters = true;

    conf.factorS = 1;
    conf.factorb = 1;
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
  virtual void stepNoLearning(const sensor*,
                              int number_sensors,
                              motor*,
                              int number_motors) override;

  /// called during babbling phase
  virtual void motorBabblingStep(const sensor*,
                                 int number_sensors,
                                 const motor*,
                                 int number_motors) override;

  /***** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const;
  /** loads the controller values from a given file. */
  virtual bool restore(FILE* f);

  /* some direct access functions (unsafe!) */
  virtual matrix::Matrix getA() const;
  virtual void setA(const matrix::Matrix& A);
  virtual matrix::Matrix getC() const;
  virtual void setC(const matrix::Matrix& C);
  virtual matrix::Matrix geth() const;
  virtual void seth(const matrix::Matrix& h);

  /***** TEACHABLE ****/
  virtual void setMotorTeaching(const matrix::Matrix& teaching) override;
  virtual void setSensorTeaching(const matrix::Matrix& teaching) override;
  virtual matrix::Matrix getLastMotorValues() override;
  virtual matrix::Matrix getLastSensorValues() override;

  /***** PARAMETRIZABLE ****/
  virtual std::list<matrix::Matrix> getParameters() const;
  virtual int setParameters(const std::list<matrix::Matrix>& params);

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;
  static constexpr unsigned short buffersize = 10;

  matrix::Matrix A;                    // Model Matrix
  matrix::Matrix C;                    // Controller Matrix
  matrix::Matrix S;                    // Model Matrix (sensor branch)
  matrix::Matrix h;                    // Controller Bias
  matrix::Matrix b;                    // Model Bias
  matrix::Matrix L;                    // Jacobi Matrix
  matrix::Matrix R;                    //
  matrix::Matrix C_native;             // Controller Matrix obtained from motor babbling
  matrix::Matrix A_native;             // Model Matrix obtained from motor babbling
  matrix::Matrix y_buffer[buffersize]; // buffer needed for delay
  matrix::Matrix x_buffer[buffersize]; // buffer of sensor values
  matrix::Matrix v_avg;
  matrix::Matrix x;        // current sensor value vector
  matrix::Matrix x_smooth; // time average of x values
  int t = 0;

  bool loga = false;

  SoxConf conf; ///< configuration objects

  bool intern_isTeaching = false;    // teaching signal available?
  matrix::Matrix y_teaching; // motor teaching  signal

  paramval creativity;
  paramval sense;
  paramval harmony;
  paramval causeaware;
  paramint pseudo;
  paramval epsC;
  paramval epsA;
  paramval damping;
  paramval gamma; // teaching strength

  void constructor();

  // calculates the pseudo inverse of L in different ways, depending on pseudo
  matrix::Matrix pseudoInvL(const matrix::Matrix& L,
                            const matrix::Matrix& A,
                            const matrix::Matrix& C);

  /// learn values model and controller (A,b,C,h)
  virtual void learn();

  /// neuron transfer function
  static double g(double z) {
    return tanh(z);
  };

  /// derivative of g
  static double g_s(double z) {
    double k = tanh(z);
    return 1.0 - k * k;
  };

  /// function that clips the second argument to the interval [-first,first]
  static constexpr double clip(double r, double x) {
    return x < -r ? -r : (x > r ? r : x);
  }
  /// calculates the inverse the argument (useful for Matrix::map)
  static constexpr double one_over(double x) {
    return 1 / x;
  }
};

#endif
