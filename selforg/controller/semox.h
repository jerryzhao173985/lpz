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
#ifndef __SEMOX_H
#define __SEMOX_H

#include <selforg/homeokinbase.h>
#include <selforg/matrix.h>
#include <selforg/noisegenerator.h>
#include <selforg/parametrizable.h>
#include <selforg/randomgenerator.h>
#include <selforg/teachable.h>
#include <vector>

struct SeMoXConf {
  unsigned short buffersize = 50;     ///< size of the ringbuffers for sensors, motors,...
  double cInit = 1.0;                 ///< init size of C (controller matrix)
  double cNonDiag = 0;                ///< size of nondiagonal elements of C (controller matrix)
  double aInit = 1.0;                 ///< init size of A (model matrix)
  double sInit = 0.0;                 ///< init size of S (sensor to sensor matrix)
  matrix::Matrix
    initialC;   ///< initialC initial controller matrix (if null matrix then automatic, see cInit)
                   ///< not given)
  bool
    modelExt = true; ///< modelExt if true then additional matrix S is used in forward model (sees sensors)
  int numContext = 0;  ///< number of context sensors (considered at the end of the sensor
                       ///< vector, which are only feed to the model extended model)
  bool
    someInternalParams = true; ///< someInternalParams if true only some internal parameters are exported
};

/**
 * This controller follows the prinziple of homeokinesis and
 *  implements the extensions described in the thesis of Georg Martius
 *  2009, University Goettingen:
 *  Goal-Oriented Control of Self-organizing Behavior in Autonomous Robots
 *
 * This class also learns the inverse model and learns bias transformation.
 *
 * Homekinetic controller using sensors and motors.
 * 
 */
class SeMoX : public HomeokinBase, public Teachable, public Parametrizable {
  friend class ThisSim;

public:
  explicit SeMoX(const SeMoXConf& conf = getDefaultConf());

  /// returns the default configuration
  static SeMoXConf getDefaultConf() {
    SeMoXConf c;
    c.buffersize = 50;
    // c.initialC // remains 0x0
    c.cInit = 1.0;
    c.cNonDiag = 0;
    c.aInit = 1.0;
    c.sInit = 0.0;
    c.modelExt = true;
    c.someInternalParams = true;
    c.numContext = 0;
    return c;
  }

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) override;

  virtual ~SeMoX() override;

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

  /**** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const;
  /** loads the controller values from a given file. */
  virtual bool restore(FILE* f);

  /**** INSPECTABLE ****/
  virtual std::list<ILayer> getStructuralLayers() const override;
  virtual std::list<IConnection> getStructuralConnections() const override;

  /**** TEACHABLE ****/
  /** The given motor teaching signal is used for this timestep.
      It is used as a feed forward teaching signal for the controller.
      Please note, that the teaching signal has to be given each timestep
       for a continuous teaching process.
     @param teaching: matrix with dimensions (motornumber,1)
   */
  virtual void setMotorTeaching(const matrix::Matrix& teaching) override;

  /** The given sensor teaching signal (distal learning) is used for this timestep.
      The belonging motor teachung signal is calculated by the inverse model.
      See setMotorTeaching
     @param teaching: matrix with dimensions (motorsensors,1)
   */
  virtual void setSensorTeaching(const matrix::Matrix& teaching) override;
  /// returns the last motor values (useful for cross motor coupling)
  virtual matrix::Matrix getLastMotorValues() override;
  /// returns the last sensor values (useful for cross sensor coupling)
  virtual matrix::Matrix getLastSensorValues() override;

  /***** PARAMETRIZABLE ****/
  virtual std::list<matrix::Matrix> getParameters() const override;
  virtual int setParameters(const std::list<matrix::Matrix>& params) override;

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;

  matrix::Matrix A;       ///< Model Matrix (motors to sensors)
  matrix::Matrix S;       ///< additional Model Matrix (sensors derivatives to sensors)
  matrix::Matrix C;       ///< Controller Matrix
  matrix::Matrix H;       ///< Controller Bias
  matrix::Matrix B;       ///< Model Bias
  matrix::Matrix R;       ///< C*A
  matrix::Matrix SmallID; ///< small identity matrix in the dimension of R
  matrix::Matrix v;       ///< shift
  matrix::Matrix xsi;     ///< current output error

  NoiseGenerator* BNoiseGen; ///< Noisegenerator for noisy bias
  paramval modelNoise;       ///< strength of noisy bias

  double xsi_norm = 0;     ///< norm of matrix
  double xsi_norm_avg = 0; ///< average norm of xsi (used to define whether Modell learns)
  double pain;         ///< if the modelling error static_cast<xsi>(is) too high we have a pain signal

  std::vector<matrix::Matrix> x_buffer;
  std::vector<matrix::Matrix> x_c_buffer; ///< buffer for sensors with context sensors
  std::vector<matrix::Matrix> y_buffer;

  matrix::Matrix y_teaching; ///< motor teaching  signal

  paramval gamma_cont;  ///< parameter to include contiuity in motor values (avoid high frequencies)
  paramval gamma_teach; ///< strength of teaching
  paramval discountS;   ///< discount strength for hierachical model

  paramval dampModel;      ///< damping of A and S matrices
  paramval dampController; ///< damping of C matrix

  SeMoXConf conf;

  // internal
  bool intern_useTeaching = false; ///< flag whether there is an actual teachning signal or not
  int t_rand = 0;             ///< initial random time to avoid syncronous management of all controllers
  int managementInterval = 0; ///< interval between subsequent management function calls
  parambool _modelExt_copy; ///< copy of modelExtension variable (to achieve readonly)

  /// puts the sensors in the ringbuffer, generate controller values and put them in the
  //  ringbuffer as well
  virtual void fillBuffersAndControl(const sensor* x_,
                                     int number_sensors,
                                     motor* y_,
                                     int number_motors);

  /// calculates xsi for the current time step using the delayed y values
  //  and x delayed by one
  //  @param delay 0 for no delay and n>0 for n timesteps delay in the time loop
  virtual void calcXsi(int delay);

  /// learn H,C with motors y and corresponding sensors x
  virtual void learnController();

  /// learn A, (and S) using motors y and corresponding sensors x
  //  @param delay 0 for no delay and n>0 for n timesteps delay in the time loop
  virtual void learnModel(int delay);

  /// calculates the predicted sensor values
  virtual matrix::Matrix model(const std::vector<matrix::Matrix>& x_buffer, int delay, const matrix::Matrix& y);

  /// handles inhibition damping etc.
  virtual void management();

  /// returns controller output for given sensor values
  virtual matrix::Matrix calculateControllerValues(const matrix::Matrix& x_smooth);

protected:
  static double regularizedInverse(double v);
  
  // Helper methods for vector-based buffers (overload base class methods)
  void putInBuffer(std::vector<matrix::Matrix>& buffer, const matrix::Matrix& vec, int delay = 0) {
    buffer[(t - delay) % buffersize] = vec;
  }
  
  matrix::Matrix calculateSmoothValuesVec(const std::vector<matrix::Matrix>& buffer, int number_steps_for_averaging_) {
    // number_steps_for_averaging_ must not be larger than buffersize
    assert(static_cast<unsigned>(number_steps_for_averaging_) <= buffersize);
    
    matrix::Matrix result(buffer[t % buffersize]);
    for (int k = 1; k < number_steps_for_averaging_; ++k) {
      result += buffer[(t - k + buffersize) % buffersize];
    }
    result *= 1 / (static_cast<double>(number_steps_for_averaging_)); // scalar multiplication
    return result;
  }
  
  // Rule of 5: Delete copy operations, allow move
  SeMoX(const SeMoX&) = delete;
  SeMoX& operator=(const SeMoX&) = delete;
  SeMoX(SeMoX&&) = default;
  SeMoX& operator=(SeMoX&&) = default;
};

#endif
