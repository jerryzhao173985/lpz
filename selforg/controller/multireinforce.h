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
#ifndef __MULTIREINFORCE_H
#define __MULTIREINFORCE_H

#include <selforg/abstractcontroller.h>
#include <selforg/multilayerffnn.h>

#include <cassert>
#include <cmath>
#include <vector>
#include <memory>

#include <selforg/matrix.h>
#include <selforg/multilayerffnn.h>
#include <selforg/noisegenerator.h>
#include <selforg/qlearning.h>

struct MultiReinforceConf {
  unsigned short buffersize = 0;       ///< size of the ringbuffers for sensors, motors,...
  int numContext;                  ///< number of context sensors (ignored)
  std::list<std::string> satFiles; /// filenames for sat networks
  int numSats = 0;                     ///< number of satelite networks (derived from length of files
  bool useDerive = false;                  ///< input to sat network includes derivatives
  bool useY = false;                       ///< input to sat network includes y (motor values)
  //   double tauE1;         ///< time horizont for short averaging error
  //   double tauH;          ///< hystersis time (time an state is kept even another one seams
  //   right) double tauI;          ///< maximal waiting time for state change if action was changed
  int reinforce_interval = 0; ///<  time between consecutive reinforcement selections

  QLearning* qlearning;         ///< QLearning instance
  matrix::Matrix* actioncorrel; /// correlation matrix of actions
};

/// Satelite network struct
struct Sat {
  Sat(MultiLayerFFNN* _net, double _eps);
  MultiLayerFFNN* net;
  double eps = 0;
  double lifetime = 0;
};

/**
 * class for multi-layer neural network reinforcement learning controller
 */
class MultiReinforce : public AbstractController {

public:
  explicit MultiReinforce(const MultiReinforceConf& conf = getDefaultConf());
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) override;

  virtual ~MultiReinforce() override;

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

  /// restores the sat networks from seperate files
  static std::list<std::string> createFileList(const char* filestem, int n);
  /// restores the sat networks from seperate files
  void restoreSats(const std::list<std::string>& files);
  /// stores the sats into the given files
  void storeSats(const std::list<std::string>& files);

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

  static MultiReinforceConf getDefaultConf() {
    MultiReinforceConf c;
    c.buffersize = 10;
    c.numContext = 0;
    c.numSats = 0; // has to be changed by user!
    c.useDerive = false;
    c.useY = true;
    c.qlearning = 0;
    //     c.tauE1=25;
    //     c.tauH=10;
    //     c.tauI=50;
    c.reinforce_interval = 10;
    c.actioncorrel = 0;
    c.qlearning = 0;
    return c;
  }

protected:
  unsigned short number_sensors = 0;
  unsigned short number_motors = 0;

  // sensor, sensor-derivative and motor values storage
  unsigned short buffersize = 0;
  std::vector<matrix::Matrix> x_buffer;
  std::vector<matrix::Matrix> xp_buffer;
  std::vector<matrix::Matrix> y_buffer;
  std::vector<matrix::Matrix> x_context_buffer;

  std::vector<Sat> sats;       ///< satelite networks
  bool manualControl;          ///< True if actions static_cast<sats>(are) selected manually
  matrix::Matrix nomSatOutput; ///< norminal output of satelite networks (x_t,y_t)^T
  matrix::Matrix satInput;     ///< input to satelite networks (x_{t-1}, xp_{t-1}, y_{t-1})^T
  int action = 0;                  ///< index of controlling network
  int newaction = 0;               ///< index of new controlling network
  int oldaction = 0;               ///< index of old controlling network
  int state = 0;                   ///< current state
  double reward = 0;               ///< current reward
  double oldreward = 0;            ///< old reward (nicer for plotting)
  int phase = 0;    ///< current phase of the controller: 0: action just selected 1:state changed first
                ///< time 2:state changed second time
  int phasecnt = 0; ///< counts number of steps in one phase.

  matrix::Matrix satErrors;    ///< actual errors of the sats
  matrix::Matrix satAvgErrors; ///< averaged errors of the sats
  matrix::Matrix statesbins;   ///< bins with counts for each state

  MultiReinforceConf conf;
  bool initialised = false;
  int t = 0;
  int managementInterval = 0; ///< interval between subsequent management calls

  // Rule of 5: Delete copy operations, allow move
  MultiReinforce(const MultiReinforce&) = delete;
  MultiReinforce& operator=(const MultiReinforce&) = delete;
  MultiReinforce(MultiReinforce&&) = default;
  MultiReinforce& operator=(MultiReinforce&&) = default;

  /// returns number of state, to be overwritten
  virtual int getStateNumber() = 0;

  /// returns state, to be overwritten
  virtual int calcState() = 0;

  /// returns the reinforcement (reward), to be overwritten
  virtual double calcReinforcement() = 0;

  // put new value in ring buffer
  void putInBuffer(std::vector<matrix::Matrix>& buffer, const matrix::Matrix& vec, int delay = 0);

  /// puts the sensors in the ringbuffer
  virtual void fillSensorBuffer(const sensor* x_, int number_sensors);
  /// puts the motors in the ringbuffer
  virtual void fillMotorBuffer(const motor* y_, int number_motors);

  /// handles inhibition damping etc.
  virtual void management();

  /** Calculates first and second derivative and returns both in on matrix (above).
      We use simple discrete approximations:
      \f[ f'(x) = (f(x) - f(x-1)) / 2 \f]
      \f[ f''(x) = f(x) - 2f(x-1) + f(x-2) \f]
      where we have to go into the past because we do not have f(x+1). The scaling can be neglegted.
  */
  matrix::Matrix calcDerivatives(const std::vector<matrix::Matrix>& buffer, int delay);
};

#endif
