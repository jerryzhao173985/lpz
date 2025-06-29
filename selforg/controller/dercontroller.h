/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
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
#ifndef __DERCONTROLLER_H
#define __DERCONTROLLER_H

#include "invertmotorcontroller.h"

#include <cassert>
#include <cmath>
#include <vector>

#include "matrix.h"
#include "noisegenerator.h"

struct DerControllerConf {
  int buffersize = 0;  ///< buffersize size of the time-buffer for x,y,eta
  double cInit = 0;    ///< cInit size of the C matrix to initialised with.
  double cNonDiag = 0; ///< cNonDiag is the size of the nondiagonal elements in respect to the diagonal
                   ///< (cInit*ones)
  bool useS = false;       ///< useS decides whether to use the S matrix in addition to the A matrix
  bool someInternalParams = false; ///< someInternalParams if true only some internal parameters are
                           ///< exported, all otherwise
  bool useTeaching = false;        ///< if true, the controller honors the teaching signal
  bool useFantasy = false;         ///< if true fantasising is enabled
};

/**
 * class for robot controller that uses the Homeokinetic (Der) algorithm.
 */
class DerController : public InvertMotorController {

public:
  explicit DerController(const DerControllerConf& conf = getDefaultConf());
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override;

  virtual ~DerController() override;

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
  virtual bool store(FILE* f) const;
  virtual bool restore(FILE* f);

  /**** CONFIGURABLE ****/
  virtual std::list<iparamkey> getInternalParamNames() const override;
  virtual std::list<iparamval> getInternalParams() const override;
  virtual std::list<ILayer> getStructuralLayers() const override;
  virtual std::list<IConnection> getStructuralConnections() const override;

  /**** TEACHING ****/
  virtual void setTeachingMode(bool onOff);
  virtual bool getTeachingMode() const;
  virtual void setMotorTeachingSignal(const motor* teaching, int len);
  // void calcCandHUpdatesTeaching(const Matrix& C_update, const Matrix& H_update, int y_delay);
  // void calcCandHUpdates(const Matrix& C_update, const Matrix& H_update,const Matrix& A_update, int y_delay);//Test
  // A

  static DerControllerConf getDefaultConf() {
    DerControllerConf c;
    c.buffersize = 50;
    c.cInit = 1.2;
    c.cNonDiag = 0;
    c.useS = false;
    // c.someInternalParams = true;//This is for gnuplout, only the first few nodiagonal elements
    c.someInternalParams = false; // This is for gnuplout,to plot all matrix  elements
    c.useTeaching = false;
    c.useFantasy = false;
    return c;
  }

  void getLastMotors(motor* motors, int len);

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;

  matrix::Matrix A;          ///< Model Matrix (motors to sensors)
  matrix::Matrix S;          ///< additional Model Matrix (sensors to sensors)
  matrix::Matrix C;          ///< Controller Matrix
  matrix::Matrix DD;         ///< Noise  Matrix
  matrix::Matrix Dinverse;   ///< Inverse  Noise  Matrix
  matrix::Matrix H;          ///< Controller Bias
  matrix::Matrix B;          ///< Model Bias
  NoiseGenerator* BNoiseGen; ///< Noisegenerator for noisy bias
  NoiseGenerator* YNoiseGen; ///< Noisegenerator for noisy motor values
  matrix::Matrix R;          ///< C*A
  matrix::Matrix RRT;        // R*R^T
  matrix::Matrix AAT;        // (A^T)*A
  //  matrix::Matrix Rm1; ///< R^-1
  matrix::Matrix SmallID; ///< small identity matrix in the dimension of R
  matrix::Matrix xsi;     ///< current output error
  double xsi_norm = 0;        ///< norm of matrix
  double xsi_norm_avg = 0;    ///< average norm of xsi (used to define whether Modell learns)
  double pain;            ///< if the modelling error (xsi is) too high we have a pain signal
  std::vector<matrix::Matrix> x_buffer;
  std::vector<matrix::Matrix> y_buffer;
  std::vector<matrix::Matrix> eta_buffer;
  matrix::Matrix zero_eta; // zero initialised eta
  matrix::Matrix x_smooth;
  //  matrix::Matrix v_smooth;
  matrix::Matrix eta_smooth;

  matrix::Matrix y_teaching; ///< teaching motor signal

  matrix::Matrix x_intern; ///< fantasy sensor values
  int fantControl = 0;         ///< interval length for fantasising
  int fantControlLen = 0;      ///< length of fantasy control
  int fantReset = 0;           ///< number of fantasy control events before reseting internal state

  DerControllerConf conf;

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
  virtual void learnController();

  /// calculates the predicted sensor values
  virtual matrix::Matrix model(const matrix::Matrix& x, const matrix::Matrix& y);

  /// calculates the Update for C, H and A
  // @param y_delay timesteps to delay the y-values.  (usually 0)
  //  Please note that the delayed values are NOT used for the error calculation
  //  (this is done in calcXsi())
  virtual void calcCandHandAUpdates(matrix::Matrix& C_update,
                                    matrix::Matrix& H_update,
                                    matrix::Matrix& A_update,
                                    int y_delay); // Test A
  /// updates the matrices C, H and A
  virtual void updateCandHandA(const matrix::Matrix& C_update,
                               const matrix::Matrix& H_update,
                               const matrix::Matrix& A_update,
                               double squashSize); // Test A

  virtual matrix::Matrix calculateControllerValues(const matrix::Matrix& x_smooth);

  /// calculates the city block distance (abs norm) of the matrix. (abs sum of absolutes / size of
  /// matrix)
  virtual double calcMatrixNorm(const matrix::Matrix& m);
  /// calculates the error_factor for either logarithmic (E=ln(e^T*e)) or square (E=sqrt(e^t*e))
  /// error
  virtual double calcErrorFactor(const matrix::Matrix& e, bool loga, bool root) override;
  
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
  DerController(const DerController&) = delete;
  DerController& operator=(const DerController&) = delete;
  DerController(DerController&&) = default;
  DerController& operator=(DerController&&) = default;
};

#endif
