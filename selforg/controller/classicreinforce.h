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
#ifndef __CLASSICREINFORCE_H
#define __CLASSICREINFORCE_H

#include <selforg/abstractcontroller.h>

#include <cassert>
#include <cmath>
#include <memory>

#include <selforg/matrix.h>
#include <selforg/noisegenerator.h>
#include <selforg/qlearning.h>

struct ClassicReinforceConf {
  unsigned short buffersize = 0; ///< size of the ringbuffers for sensors, motors,...

  std::unique_ptr<QLearning> qlearning; ///< QLearning instance

  // Rule of 5 for proper RAII
  ClassicReinforceConf()
    : buffersize(0) {}
  ~ClassicReinforceConf() = default;
  ClassicReinforceConf(const ClassicReinforceConf&) = delete;
  ClassicReinforceConf& operator=(const ClassicReinforceConf&) = delete;
  ClassicReinforceConf(ClassicReinforceConf&&) = default;
  ClassicReinforceConf& operator=(ClassicReinforceConf&&) = default;
};

/**
 * Classic reinforcement learning controller.
 */
class ClassicReinforce : public AbstractController {

public:
  explicit ClassicReinforce(ClassicReinforceConf conf = getDefaultConf());
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override;

  virtual ~ClassicReinforce() override;

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
  virtual void stepNoLearning(const sensor*, int number_sensors, motor*, int number_motors) override;

  // !!!!!!!!!!!!!!!!!!! MISC STUFF !!!!!!!!

  /** enables/disables manual control, action_ is the sat network number to be used
      if mControl is false, action is ignored
   */
  void setManualControl(bool mControl, int action_ = 0);

  /************** CONFIGURABLE ********************************/
  virtual void notifyOnChange(const paramkey& key) override;

  /**** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const;
  /** loads the controller values from a given file. */
  virtual bool restore(FILE* f);

  /**** INSPECTABLE ****/
  virtual std::list<iparamkey> getInternalParamNames() const override;
  virtual std::list<iparamval> getInternalParams() const override;
  virtual std::list<ILayer> getStructuralLayers() const override;
  virtual std::list<IConnection> getStructuralConnections() const override;

  static ClassicReinforceConf getDefaultConf() {
    ClassicReinforceConf c;
    c.buffersize = 10;
    // c.numContext = 0;
    // c.reinforce_interval = 10;
    c.qlearning = nullptr;
    return c;
  }

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;

  // sensor, sensor-derivative and motor values storage
  unsigned short buffersize = 0;
  std::unique_ptr<matrix::Matrix[]> x_buffer;
  std::unique_ptr<matrix::Matrix[]> y_buffer;
  std::unique_ptr<matrix::Matrix[]> x_context_buffer;

  bool manualControl; ///< True if actions static_cast<sats>(are) selected manually

  int action = 0;       ///< action
  int oldaction = 0;    ///< old action
  int state = 0;        ///< current state
  double reward = 0;    ///< current reward
  double oldreward = 0; ///< old reward (nicer for plotting)

  ClassicReinforceConf conf;
  bool initialised = false;
  int t = 0;
  int managementInterval = 0; ///< interval between subsequent management calls
  int reinforce_interval = 10; ///< interval between reinforcement steps

  /// returns number of state, to be overwritten
  virtual int getStateNumber() = 0;

  /// returns state, to be overwritten
  virtual int calcState() = 0;

  /// returns number of actions, to be overwritten
  virtual int getActionNumber() = 0;
  /// returns action Matrix from discrete actions, to be overwritten
  virtual matrix::Matrix calcMotor(int action) = 0;

  /// returns the reinforcement (reward), to be overwritten
  virtual double calcReinforcement() = 0;

  // put new value in ring buffer
  void putInBuffer(matrix::Matrix* buffer, const matrix::Matrix& vec, int delay = 0);

  /// puts the sensors in the ringbuffer
  virtual void fillSensorBuffer(const sensor* x_, int number_sensors);
  /// puts the motors in the ringbuffer
  virtual void fillMotorBuffer(const motor* y_, int number_motors);

  /// handles inhibition damping etc.
  virtual void management();
};

#endif
