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
#ifndef __INVERTMOTORNSTEP_H
#define __INVERTMOTORNSTEP_H

#include "invertmotorcontroller.h"
#include "teachable.h"

#include <cassert>
#include <cmath>
#include <vector>

#include <selforg/matrix.h>
#include <selforg/noisegenerator.h>

struct InvertMotorNStepConf {
  int buffersize = 0;          ///< buffersize size of the time-buffer for x,y,eta
  matrix::Matrix initialC; ///< initial controller matrix C (if not given then randomly chosen, see
                           ///< cInit, cNonDiag...)
  double cInit = 0;            ///< cInit size of the C matrix to initialised with.
  double cNonDiag = 0; ///< cNonDiag is the size of the nondiagonal elements in respect to the diagonal
                   ///< (cInit) ones
  double cNonDiagAbs = 0; ///< cNonDiag is the value of the nondiagonal elements
  bool
    useS; ///< useS decides whether to use the S matrix in addition to the A matrix (sees sensors)
  /** useSD decides whether to use the SD matrix in addition to the A
      matrix (sees first and second derivatives)  */
  bool useSD = false;
  /** number of context sensors(considered at the end of the sensor
      vector. If not 0, then S will only get them as input  */
  int numberContext = 0;
  bool someInternalParams = false; ///< someInternalParams if true only some internal parameters are
                           ///< exported, all otherwise
};

/**
 * class for robot controller that uses the homeokinetic learning algorithm
 * with arbitrary extensions like teachable, motor babbling, and so on
 */
class InvertMotorNStep : public InvertMotorController, public Teachable {

public:
  explicit InvertMotorNStep(const InvertMotorNStepConf& conf = getDefaultConf());

  virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override;

  virtual ~InvertMotorNStep() override;

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

  /**** TEACHING ****/
  /** The given motor teaching signal is used for this timestep.
      It is used as a feed forward teaching signal for the controller.
      Please note, that the teaching signal has to be given each timestep
       for a continuous teaching process.
   */
  virtual void setMotorTeachingSignal(const motor* teaching, int len);

  /** The given sensor teaching signal (distal learning) is used for this timestep.
      First the belonging motor teachung signal is calculated by the inverse model.
      See setMotorTeachingSignal
   */
  virtual void setSensorTeachingSignal(const sensor* teaching, int len);
  void getLastMotors(motor* motors, int len);
  void getLastSensors(sensor* sensors, int len);

  /**** New TEACHING interface ****/
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

  // UNUSED! OLD IMPLEMENTATION which hat some consistency arguments
  void calcCandHUpdatesTeaching(matrix::Matrix& C_update, matrix::Matrix& H_update, int y_delay);

  /**** REINFORCEMENT ****/
  /** set the reinforcement signal for this timestep.
      It is used to calculate a factor for the update.
      Factor = 1-0.95*reinforcement.
      @param reinforcement value between -1 and 1 (-1 bad, 0 neutral, 1 good)
   */
  virtual void setReinforcement(double reinforcement);

  static InvertMotorNStepConf getDefaultConf() {
    InvertMotorNStepConf c;
    c.buffersize = 50;
    c.cInit = 1.0;
    c.cNonDiag = 0;
    c.useS = false;
    c.useSD = false;
    c.numberContext = 0;
    c.someInternalParams = true;
    c.cNonDiagAbs = 0.0;
    return c;
  }

  /// sets the sensor channel weights (matrix should be (getSensorNumber() x 1)
  void setSensorWeights(const matrix::Matrix& weights);
  matrix::Matrix getSensorWeights() const {
    return sensorweights;
  }
  /// reference to C-matrix
  const matrix::Matrix& getC() const {
    return C;
  };

  double getE() const {
    return v.multTM().val(0, 0);
  }

protected:
  unsigned short number_sensors;
  unsigned short number_motors;

public:
  matrix::Matrix A;  ///< Model Matrix (motors to sensors)
  matrix::Matrix S;  ///< additional Model Matrix (sensors to sensors)
  matrix::Matrix SD; ///< additional Model Matrix (sensors derivatives to sensors)
  matrix::Matrix C;  ///< Controller Matrix
  matrix::Matrix H;  ///< Controller Bias
  matrix::Matrix B;  ///< Model Bias
protected:
  NoiseGenerator* BNoiseGen; ///< Noisegenerator for noisy bias
  NoiseGenerator* YNoiseGen; ///< Noisegenerator for noisy motor output
  matrix::Matrix R;          ///< C*A
  matrix::Matrix SmallID;    ///< small identity matrix in the dimension of R
  matrix::Matrix xsi;        ///< current output error
  matrix::Matrix v;          ///< current reconstructed error
  double E_val;              ///< value of Error function
  double xsi_norm;           ///< norm of matrix
  double xsi_norm_avg;       ///< average norm of xsi (used to define whether Modell learns)
  double pain;               ///< if the modelling error (xsi) is too high we have a pain signal
  std::vector<matrix::Matrix> x_buffer;
  std::vector<matrix::Matrix> y_buffer;
  std::vector<matrix::Matrix> eta_buffer;
  matrix::Matrix zero_eta; // zero initialised eta
  matrix::Matrix x_smooth;
  //   matrix::Matrix z; ///< membrane potential
  matrix::Matrix y_teaching; ///< teaching motor signal
  bool useTeaching;          ///< flag whether there is an actual teachning signal or not
  double reinforcement;      ///< reinforcement value (set via setReinforcement())
  double reinforcefactor;    ///< reinforcement factor (set to 1 every step after learning)
  int t_rand; ///< initial random time to avoid syncronous management of all controllers

  matrix::Matrix sensorweights; ///< sensor channel weight (each channel gets a certain importance)

  /** factor to teach for continuity: subsequent motor commands
      should not differ too much */
  double continuity;
  double modelCompliant;  ///< learning factor for model (or sensor) compliant learning
  int managementInterval; ///< interval between subsequent management function calls
  paramval inhibition;    ///< inhibition strength for sparce kwta strategy (is scaled with epsC)
  paramval kwta;          ///< static_cast<int> number of synapses that get strengthend
  paramval limitRF; ///< static_cast<int> receptive field of motor neurons (number of offcenter sensors) if
                    ///< null then no limitation. Mutual exclusive with inhibition
  paramval dampS;   ///< damping of S matrix
  paramval dampC;   ///< damping of C matrix
  paramval activeExplore; ///< decides whether and how strong the backpropagated error is used as a
                          ///< control signal
  paramval cfactor;
  paramval cnondiagabs;
  paramval cdiagabs;

  InvertMotorNStepConf conf;

  /// puts the sensors in the ringbuffer, generate controller values and put them in the
  //  ringbuffer as well
  virtual void fillBuffersAndControl(const sensor* x_,
                                     int number_sensors,
                                     motor* y_,
                                     int number_motors);

  /// calculates the first shift into the motor space useing delayed motor values.
  //  @param delay 0 for no delay and n>0 for n timesteps delay in the time loop
  virtual void calcEtaAndBufferIt(int delay);
  /// calculates xsi for the current time step using the delayed y values
  //  and x delayed by one
  //  @param delay 0 for no delay and n>0 for n timesteps delay in the time loop
  virtual void calcXsi(int delay);

  /// learn H,C with motors y and corresponding sensors x
  //  @param delay 0 for no delay and n>0 for n timesteps delay in the time loop
  virtual void learnController(int delay);

  /// calculates the Update for C and H
  // @param delay timesteps to delay the y-values.  (usually 0)
  //  Please note that the delayed values are NOT used for the error calculation
  //  (this is done in calcXsi())
  virtual void calcCandHUpdates(matrix::Matrix& C_update, matrix::Matrix& H_update, int delay);

  /// updates the matrix C and H
  virtual void updateCandH(const matrix::Matrix& C_update,
                           const matrix::Matrix& H_update,
                           double squashSize);

  /// learn A, (and S) using motors y and corresponding sensors x
  //  @param delay 0 for no delay and n>0 for n timesteps delay in the time loop
  virtual void learnModel(int delay);

  /// calculates the predicted sensor values
  virtual matrix::Matrix model(const std::vector<matrix::Matrix>& x_buffer, int delay, const matrix::Matrix& y);

  /// handles inhibition damping etc.
  virtual void management();

  /// returns controller output for given sensor values
  virtual matrix::Matrix calculateControllerValues(const matrix::Matrix& x_smooth);
  
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

  /** Calculates first and second derivative and returns both in on matrix (above).
      We use simple discrete approximations:
      \f[ f'(x) = (f(x) - f(x-1)) / 2 \f]
      \f[ f''(x) = f(x) - 2f(x-1) + f(x-2) \f]
      where we have to go into the past because we do not have f(x+1). The scaling can be neglegted.
  */
  matrix::Matrix calcDerivatives(const std::vector<matrix::Matrix>& buffer, int delay);

public:
  /** k-winner take all inhibition for synapses. k largest synapses are strengthed and the rest are
     inhibited. strong synapes are scaled by 1+(damping/k) and weak synapses are scaled by
     1-(damping/(n-k)) where n is the number of synapes
      @param weightmatrix reference to weight matrix. Synapses for a neuron are in one row.
             The inhibition is done for all rows independently
      @param k number of synapes to strengthen
      @param damping strength of supression and exitation (typically 0.001)
   */
  void kwtaInhibition(matrix::Matrix& weightmatrix, unsigned int k, double damping);

  /** sets all connections to zero which are further away then rfSize
      from the diagonal.
      If rfSize == 1 then only main diagonal is left.
      If rfSize = 2: main diagonal and upper and lower side diagonal are kept and so on and so
     forth.
   */
  void limitC(matrix::Matrix& weightmatrix, unsigned int rfSize);

  static double clip095(double x);
  static double regularizedInverse(double v);
  
  // Rule of 5: Delete copy operations, allow move
  InvertMotorNStep(const InvertMotorNStep&) = delete;
  InvertMotorNStep& operator=(const InvertMotorNStep&) = delete;
  InvertMotorNStep(InvertMotorNStep&&) = default;
  InvertMotorNStep& operator=(InvertMotorNStep&&) = default;
};

#endif
