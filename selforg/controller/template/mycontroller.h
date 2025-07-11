/***************************************************************************
 *   Copyright (C) 2025 by Your Name                                       *
 *   your.email@example.com                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MYCONTROLLER_H
#define __MYCONTROLLER_H

#include <selforg/abstractcontroller.h>
#include <selforg/matrix.h>
#include <selforg/teachable.h>

/**
 * Template for a new self-organizing controller
 * 
 * This controller implements [describe your approach here]
 * 
 * Key features:
 * - Homeokinetic learning 
 * - Stability mechanisms
 * - Parameter adaptation
 * 
 * Parameters:
 * - epsA: learning rate for model (default: 0.05)
 * - epsC: learning rate for controller (default: 0.05)
 * - damping: stability damping factor (default: 0.01)
 * 
 * Based on: Der & Martius "The Playful Machine" (2011)
 */
class MyController : public AbstractController, public Teachable {
public:
  /**
   * Constructor
   * @param init_feedback_strength initial strength of feedback connections
   * @param use_logarithmic whether to use logarithmic error (smoother learning)
   */
  MyController(double init_feedback_strength = 1.0, bool use_logarithmic = true);
  
  virtual ~MyController();
  
  /// Initializes controller with given sensor/motor numbers
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) override;
  
  /// Performs one control step
  virtual void step(const sensor* sensors, int number_sensors,
                    motor* motors, int number_motors) override;
  
  /// Performs one step without learning (test mode)
  virtual void stepNoLearning(const sensor* sensors, int number_sensors,
                               motor* motors, int number_motors) override;
  
  /// Returns number of sensors
  virtual int getSensorNumber() const override { return number_sensors; }
  
  /// Returns number of motors  
  virtual int getMotorNumber() const override { return number_motors; }
  
  /// Returns true if controller was initialized
  virtual bool isInitialized() const { return initialized; }
  
  /************** TEACHABLE INTERFACE **************/
  virtual void setMotorTeaching(const motor* teaching, int len) override;
  virtual void setSensorTeaching(const sensor* teaching, int len) override;
  virtual motor* getLastMotorValues() override { return y_buffer; }
  virtual sensor* getLastSensorValues() override { return x_buffer; }
  
  /************** STOREABLE INTERFACE **************/
  virtual bool store(FILE* f) const override;
  virtual bool restore(FILE* f) override;
  
  /************** INSPECTABLE INTERFACE ************/
  virtual std::list<matrix::Matrix> getInternalState() const override;
  virtual std::list<std::string> getInternalStateNames() const override;
  
  /************** CONFIGURABLE INTERFACE ***********/
  virtual paramval getParam(const paramkey& key, bool traverseChildren = true) const override;
  virtual bool setParam(const paramkey& key, paramval val, bool traverseChildren = true) override;
  virtual paramlist getParamList() const override;
  
protected:
  /// Performs learning update
  virtual void learn();
  
  /// Calculates controller outputs from inputs  
  virtual matrix::Matrix calculateControllerValues(const matrix::Matrix& x);
  
  /// Updates internal model
  virtual void updateModel();
  
  /// Checks matrix stability and applies corrections if needed
  virtual void ensureStability();
  
  /// Clips values to [-1, 1] range
  static double clip(double x) { return x > 1.0 ? 1.0 : (x < -1.0 ? -1.0 : x); }
  
  /// Activation function
  static double g(double x) { return tanh(x); }
  
  /// Derivative of activation function
  static double g_s(double x) { double y = tanh(x); return 1.0 - y*y; }
  
protected:
  // Controller matrices
  matrix::Matrix A;  ///< Model matrix (motors to sensors)
  matrix::Matrix C;  ///< Controller matrix (sensors to motors)
  matrix::Matrix S;  ///< Noise matrix
  matrix::Matrix h;  ///< Bias vector
  matrix::Matrix b;  ///< Model bias
  
  // Learning parameters
  paramval epsA;     ///< Learning rate for A matrix
  paramval epsC;     ///< Learning rate for C matrix  
  paramval damping;  ///< Damping factor for stability
  paramval noise;    ///< Noise strength
  
  // Options
  bool useLogarithmic;  ///< Use logarithmic error
  bool useTeaching;     ///< Teaching mode active
  
  // State variables
  matrix::Matrix x;  ///< Current sensor values
  matrix::Matrix y;  ///< Current motor values
  matrix::Matrix x_pred;  ///< Predicted sensor values
  matrix::Matrix xi;  ///< Prediction error
  
  // Buffers for delay
  matrix::Matrix x_buffer[2];  ///< Sensor history
  matrix::Matrix y_buffer[2];  ///< Motor history
  sensor* x_intern;  ///< Internal sensor buffer
  motor* y_intern;   ///< Internal motor buffer
  
  // Teaching signals
  const motor* y_teaching;
  const sensor* x_teaching;
  
  // System properties
  int number_sensors;
  int number_motors;
  int t;  ///< Time step counter
  bool initialized;
  
  // Constants
  static constexpr double regularization = 0.0001;  ///< Regularization for matrix inversion
};

#endif // __MYCONTROLLER_H