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
#ifndef __INVERTMOTORSPACE_H
#define __INVERTMOTORSPACE_H

#include "invertmotorcontroller.h"
#include <cassert>
#include <cmath>
#include <vector>

#include <selforg/matrix.h>
#include <selforg/noisegenerator.h>

/**
 * class for motor space inversion
 */
class InvertMotorSpace : public InvertMotorController {

public:
  explicit InvertMotorSpace(int buffersize, double cInit = 0.1, bool someInternalParams = true);
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override;

  virtual ~InvertMotorSpace() override;

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
  /** stores the controller values to a given file (binary).  */
  virtual bool store(FILE* f) const;
  /** loads the controller values from a given file (binary). */
  virtual bool restore(FILE* f);

  // inspectable interface
  virtual std::list<ILayer> getStructuralLayers() const override;
  virtual std::list<IConnection> getStructuralConnections() const override;

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;

  matrix::Matrix A;          // Model Matrix
  matrix::Matrix C;          // Controller Matrix
  matrix::Matrix R;          // C*A
  matrix::Matrix H;          // Controller Bias
  matrix::Matrix B;          // Model Bias
  NoiseGenerator* BNoiseGen; // Noisegenerator for noisy bias
  std::vector<matrix::Matrix> x_buffer;
  std::vector<matrix::Matrix> y_buffer;
  matrix::Matrix x_smooth;

  bool someInternalParams = false;
  double cInit = 0;

  /// puts the sensors in the ringbuffer, generate controller values and put them in the
  //  ringbuffer as well
  void fillBuffersAndControl(const sensor* x_, int number_sensors, motor* y_, int number_motors);

  /// learn h,C, delayed motors y and corresponding sensors x
  virtual void learnController(const matrix::Matrix& x, const matrix::Matrix& x_smooth, int delay);

  /// learn A, using motors y and corresponding sensors x
  virtual void learnModel(const matrix::Matrix& x, const matrix::Matrix& y);

  /// returns controller output for given sensor values
  virtual matrix::Matrix calculateControllerValues(const matrix::Matrix& x_smooth);

  // Helper methods for vector-based buffers
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
  InvertMotorSpace(const InvertMotorSpace&) = delete;
  InvertMotorSpace& operator=(const InvertMotorSpace&) = delete;
  InvertMotorSpace(InvertMotorSpace&&) = default;
  InvertMotorSpace& operator=(InvertMotorSpace&&) = default;
};

#endif
