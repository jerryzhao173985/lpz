/***************************************************************************
 *   Copyright (C) 2005-2011 by                                            *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
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
 *                                                                         *
 ***************************************************************************/
#ifndef __DERINF_H
#define __DERINF_H

#include "invertmotorcontroller.h"

#include <cassert>
#include <cmath>
#include <vector>

#include "invertablemodel.h"
#include "matrix.h"
#include "multilayerffnn.h"
#include "noisegenerator.h"

struct DerInfConf {
  int buffersize = 0;  ///< buffersize size of the time-buffer for x,y,eta
  double cInit = 0;    ///< cInit size of the C matrix to initialised with.
  double cNonDiag = 0; ///< cNonDiag is the size of the nondiagonal elements in respect to the diagonal
                   ///< static_cast<cInit>(ones)
  bool modelInit = false;  ///< size of the unit-map strenght of the model
  bool useS = false;       ///< useS decides whether to use the S matrix in addition to the A matrix
  bool someInternalParams = false; ///< someInternalParams if true only some internal parameters are
                           ///< exported, otherwise all
};
/**
 * class for robot controller based on information theoretic principles (Der & Martius)
 */
class DerInf : public InvertMotorController, public Storeable {

public:
  explicit DerInf(const DerInfConf& conf = getDefaultConf());
  virtual void init(int sensornumber, int motornumber, RandGen* randg) override;

  virtual ~DerInf() override;

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

  /**************  STOREABLE **********************************/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const override;
  /** loads the controller values from a given file. */
  virtual bool restore(FILE* f) override;

  /************** INSPECTABLE ********************************/
  virtual std::list<iparamkey> getInternalParamNames() const override;
  virtual std::list<iparamval> getInternalParams() const override;
  virtual std::list<ILayer> getStructuralLayers() const override;
  virtual std::list<IConnection> getStructuralConnections() const override;

  static DerInfConf getDefaultConf() {
    DerInfConf c;
    c.buffersize = 50;
    c.cInit = 1.05;
    c.cNonDiag = 0;
    c.modelInit = 1.0;
    c.someInternalParams = true;
    //   c.someInternalParams = false;
    c.useS = false;
    return c;
  }

  void getLastMotors(motor* motors, int len);

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;

  matrix::Matrix A; ///< Model Matrix (motors to sensors)
  matrix::Matrix A0;
  matrix::Matrix A_Hat;      ///< Model Matrix (motors to sensors) with input shift
  matrix::Matrix S;          ///< additional Model Matrix (sensors to sensors)
  matrix::Matrix C;          ///< Controller Matrix
  matrix::Matrix GSC;        ///< G_Prime times Controller Matrix
  matrix::Matrix DD;         ///< Noise  Matrix
  matrix::Matrix DD1;        ///< Noise  Matrix
  matrix::Matrix Dinverse;   ///< Inverse  Noise  Matrix
  matrix::Matrix H;          ///< Controller Bias
  matrix::Matrix HY;         ///< Controller Bias-Y
  matrix::Matrix B;          ///< Model Bias
  NoiseGenerator* BNoiseGen; ///< Noisegenerator for noisy bias
  NoiseGenerator* YNoiseGen; ///< Noisegenerator for noisy motor values
  matrix::Matrix R;          ///< C*A
  matrix::Matrix P;          ///< A*G'*C
  matrix::Matrix RG;         ///< Granger1
  matrix::Matrix Q;          ///< Granger2
  matrix::Matrix Q1;         //<Granger3
  matrix::Matrix RRT_inv;    // (R*R^T)^-1
  matrix::Matrix ATA_inv;    // ((A^T)*A)^-1
  matrix::Matrix Rm1;        ///< R^-1
  matrix::Matrix ID;         ///< identity matrix in the dimension of R
  matrix::Matrix ID_Sensor;  ///< identity matrix in the dimension of sensor space
  matrix::Matrix CCT_inv;
  matrix::Matrix CST;
  matrix::Matrix xsi;  ///< current output error
  double xsi_norm = 0;     ///< norm of matrix
  double xsi_norm_avg = 0; ///< average norm of xsi (used to define whether Modell learns)
  double pain;         ///< if the modelling error static_cast<xsi>(is) too high we have a pain signal
  double TLE = 0;          // TimeLoopError
  double grang1 = 0;       // GrangerCausality
  double grang2 = 0;       // GrangerCausality
  double causal = 0;       // GrangerCausality
  double causalfactor = 0; // GrangerCausality
  double EE = 0;
  double EE_mean = 0;
  double EE_sqr = 0;
  double xsistrength = 0;
  double sense = 0;
  std::vector<matrix::Matrix> x_buffer;
  std::vector<matrix::Matrix> y_buffer;
  std::vector<matrix::Matrix> ysat_buffer;
  std::vector<matrix::Matrix> chi_buffer;
  std::vector<matrix::Matrix> rho_buffer;
  std::vector<matrix::Matrix> eta_buffer;
  std::vector<matrix::Matrix> xsi_buffer;
  matrix::Matrix eta;
  matrix::Matrix ups;
  matrix::Matrix v_smooth;
  matrix::Matrix zero_eta; // zero initialised eta
  matrix::Matrix x_smooth;
  matrix::Matrix y_smooth;
  matrix::Matrix y_sat;
  matrix::Matrix y_smooth_long;
  matrix::Matrix eta_smooth;
  matrix::Matrix x_smooth_long;
  matrix::Matrix y_forecast;
  matrix::Matrix y_integration;
  matrix::Matrix y_error;
  matrix::Matrix y_intern;
  matrix::Matrix PID_deriv;
  matrix::Matrix C_updateOld;
  matrix::Matrix H_updateOld;
  matrix::Matrix HY_updateOld;
  matrix::Matrix A_updateOld;
  matrix::Matrix B_updateOld;
  matrix::Matrix L;
  matrix::Matrix G;
  matrix::Matrix xx;
  matrix::Matrix yy;
  matrix::Matrix zz;

  matrix::Matrix vau_avg;

  matrix::Matrix y_teaching; ///< teaching motor signal
  bool useTeaching = false;          ///< flag whether there is an actual teachning signal or not

  matrix::Matrix x_intern;
  int num_iterations = 0;

  int t_rand = 0; ///< initial random time to avoid syncronous management of all controllers
  int t_delay = 0;
  int managementInterval = 0;     ///< interval between subsequent management function calls
  paramval dampS;             ///< damping of S matrix
  paramval dampC;             ///< damping of C matrix
  paramval dampH;             ///< damping of H vector
  paramval weighting;         ///< general weighting factor between update concepts
  paramval epsSat;            ///< learning rate for satellite network
  paramval satelliteTeaching; ///< teaching rate for sat teaching
  paramval PIDint;
  paramval PIDdrv;
  paramval intstate;
  paramval gamma;
  paramval adaptrate;
  paramval xsifactor;
  paramval creat;

  DerInfConf conf;

  /// puts the sensors in the ringbuffer, generate controller values and put them in the
  //  ringbuffer as well
  virtual void fillBuffersAndControl(const sensor* x_,
                                     int number_sensors,
                                     motor* y_,
                                     int number_motors);

  /** learn values H,C
      This is the implementation uses a better formula for g^-1 using Mittelwertsatz
      @param delay 0 for no delay and n>0 for n timesteps delay in the SML (s4delay)
  */
  virtual void learnController(int delay);

  /// handles inhibition damping etc.
  virtual void management();

  /// returns controller output for given sensor values
  virtual matrix::Matrix calculateControllerValues(const matrix::Matrix& x_smooth);

  /** Calculates first and second derivative and returns both in on matrix (above).
      We use simple discrete approximations:
      \f[ f'(x) = (f(x) - f(x-1)) / 2 \f]
      \f[ f''(x) = f(x) - 2f(x-1) + f(x-2) \f]
      where we have to go into the past because we do not have f(x+1). The scaling can be neglegted.
  */
  matrix::Matrix calcDerivatives(const std::vector<matrix::Matrix>& buffer, int delay);

public:
  /// calculates the city block distance static_cast<abs>(norm) of the matrix. (abs sum of absolutes / size of
  /// matrix)
  virtual double calcMatrixNorm(const matrix::Matrix& m);
  
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
  DerInf(const DerInf&) = delete;
  DerInf& operator=(const DerInf&) = delete;
  DerInf(DerInf&&) = default;
  DerInf& operator=(DerInf&&) = default;
};

#endif
