/***************************************************************************
 *   Copyright (C) 2013 by                                                 *
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
#ifndef __PIMAX_H
#define __PIMAX_H

#include <selforg/abstractcontroller.h>
#include <selforg/controller_misc.h>

#include <cassert>
#include <cmath>
#include <vector>

#include <selforg/matrix.h>
#include <selforg/parametrizable.h>
#include <selforg/teachable.h>

/// configuration object for PiMax controller. Use PiMax::getDefaultConf().
struct PiMaxConf {
  double initFeedbackStrength = 0; ///< initial strength of sensor to motor connection
  bool useExtendedModel = false;       ///< if true, the extended model (S matrix) is used
  /// if true, the covariance matrix is learned otherwise a unit matrix is used
  bool useSigma = false;
  /// if true the controller can be taught see teachable interface
  bool useTeaching = false;
  /// # of steps the sensors are averaged (1 means no averaging)
  int steps4Averaging = 0;
  /// # of steps the motor values are delayed (1 means no delay)
  int steps4Delay = 0;
  bool someInternalParams = false; ///< if true only some internal parameters are exported
  bool onlyMainParameters = false; ///< if true only some configurable parameters are exported
};

/**
 * This controller implements the predictive information maximization
   described in paper: to be published in PLoS ONE 2013
   ArXiv preprint: http:__PLACEHOLDER_10__

   Note: the notation is for the model matrices is different than in the paper:
   A -> V
   S -> T
   The code contains more functionality than is described in the paper
     e.g. the teaching and motor babbling is not used.
*/
class PiMax : public AbstractController, public Teachable, public Parametrizable {

public:
  explicit PiMax(const PiMaxConf& conf = getDefaultConf());

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) override;

  virtual ~PiMax() override;

  static PiMaxConf getDefaultConf() {
    PiMaxConf conf;
    conf.initFeedbackStrength = 1.0;
    conf.useExtendedModel = false;
    conf.useSigma = true;
    conf.useTeaching = false;
    conf.steps4Averaging = 1;
    conf.steps4Delay = 1;
    conf.someInternalParams = false;
    conf.onlyMainParameters = true;
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
  virtual matrix::Matrix getA();
  virtual void setA(const matrix::Matrix& A);
  virtual matrix::Matrix getC();
  virtual void setC(const matrix::Matrix& C);
  virtual matrix::Matrix geth();
  virtual void seth(const matrix::Matrix& h);

  /***** TEACHABLE ****/
  virtual void setMotorTeaching(const matrix::Matrix& teaching) override;
  virtual void setSensorTeaching(const matrix::Matrix& teaching) override;
  virtual matrix::Matrix getLastMotorValues() override;
  virtual matrix::Matrix getLastSensorValues() override;

  /***** PARAMETRIZABLE ****/
  virtual std::list<matrix::Matrix> getParameters() const override;
  virtual int setParameters(const std::list<matrix::Matrix>& params) override;

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;
  static constexpr unsigned short buffersize = 20;

  matrix::Matrix A; // Model Matrix
  matrix::Matrix C; // Controller Matrix
  matrix::Matrix S; // Model Matrix (sensor branch)
  matrix::Matrix h; // Controller Bias
  matrix::Matrix b; // Model Bias
  matrix::Matrix L; // Jacobi Matrix

  matrix::Matrix Sigma; // noise covariance matrix

  matrix::Matrix ds0;      //
  matrix::Matrix C_native; // Controller Matrix obtained from motor babbling
  matrix::Matrix A_native; // Model Matrix obtained from motor babbling

  std::vector<matrix::Matrix> a_buffer;  // buffer needed for delay
  std::vector<matrix::Matrix> s_buffer;  // buffer of sensor values
  std::vector<matrix::Matrix> xi_buffer; // buffer of pred errors
  std::vector<matrix::Matrix> gs_buffer; // buffer of g'
  std::vector<matrix::Matrix> L_buffer;  // buffer of Jacobians

  matrix::Matrix s;        // current sensor value vector
  matrix::Matrix s_smooth; // time average of s values
  PiMaxConf conf;          ///< configuration objects

  int t = 0;

  bool intern_isTeaching = false;    // teaching signal available?
  matrix::Matrix a_teaching; // motor teaching  signal

  bool useMetric = false;
  AbstractController::paramval causeaware;
  AbstractController::paramval sense;
  AbstractController::paramval epsC;
  AbstractController::paramval epsA;
  AbstractController::paramval epsSigma;
  AbstractController::paramval factorH;
  AbstractController::paramval damping;
  AbstractController::paramval gamma; // teaching strength

  AbstractController::paramint tau; // length of time window

  /// learn values model and controller (A,b,C,h)
  virtual void learn();

  /// neuron transfer function
  static double g(double z) {
    return tanh(z);
  };

  /// derivative of g
  static double g_s(double z) {
    double k = tanh(z);
    return 1.05 - k * k; // regularized
  };

  /// function that clips the second argument to the interval [-first,first]
  static double clip(double r, double x) {
    return min(max(x, -r), r);
  }
  /// calculates the inverse the argument (useful for Matrix::map)
  static double one_over(double x) {
    return 1 / x;
  }

  // Rule of 5: Delete copy operations, allow move
  PiMax(const PiMax&) = delete;
  PiMax& operator=(const PiMax&) = delete;
  PiMax(PiMax&&) = default;
  PiMax& operator=(PiMax&&) = default;
};

#endif
