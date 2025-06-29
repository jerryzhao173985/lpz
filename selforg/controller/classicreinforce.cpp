/***************************************************************************
 *   Copyright (C) 2005-2011 by                                            *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http:__PLACEHOLDER_20__
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/

#include "classicreinforce.h"
#include <selforg/controller_misc.h>

using namespace matrix;
using namespace std;

ClassicReinforce::ClassicReinforce(ClassicReinforceConf _conf)
  : AbstractController("ClassicReinforce", "$Id: ")
  , number_sensors(0)
  , number_motors(0)
  , buffersize(_conf.buffersize)
  , x_buffer(nullptr)
  , y_buffer(nullptr)
  , x_context_buffer(nullptr)
  , manualControl(false)
  , action(0)
  , oldaction(0)
  , state(0)
  , reward(0)
  , oldreward(0)
  , conf(std::move(_conf))
  , initialised(false)
  , t(0)
  , managementInterval(100) {
  assert(conf.qlearning);

  addParameterDef("mancontrol", &manualControl, false, "Manual control used, see action");
  addParameterDef("action", &action, 0, "action to be used in manual control");
  addParameter("interval", &reinforce_interval, "interval between reinforcement steps");
};

ClassicReinforce::~ClassicReinforce() {
  // Smart pointers automatically handle cleanup
}

void
ClassicReinforce::init(int sensornumber, int motornumber, RandGen* randGen) {
  // if(!randGen) randGen = new RandGen(); // this gives a small memory leak
  number_motors = motornumber;
  number_sensors = sensornumber;
  int number_real_sensors = number_sensors - 0 /* conf.numContext */;

  x_buffer = std::make_unique<Matrix[]>(buffersize);
  y_buffer = std::make_unique<Matrix[]>(buffersize);
  x_context_buffer = std::make_unique<Matrix[]>(buffersize);
  for (unsigned int k = 0; k < buffersize; ++k) {
    x_buffer[k].set(number_real_sensors, 1);
    y_buffer[k].set(number_motors, 1);
    x_context_buffer[k].set(0 /* conf.numContext */, 1);
  }
  assert(conf.qlearning && "Please set qlearning in controller configuration");
  conf.qlearning->init(getStateNumber(), getActionNumber());

  action = 0;
  oldaction = 0;
  state = 0;
  reward = 0;
  oldreward = 0;
  t = 0;
  initialised = true;
}

// put new value in ring buffer
void
ClassicReinforce::putInBuffer(matrix::Matrix* buffer, const matrix::Matrix& vec, int delay) {
  buffer[(t - delay) % buffersize] = vec;
}

/// performs one step (includes learning). Calculates motor commands from sensor inputs.
void
ClassicReinforce::step(const sensor* x_, int number_sensors, motor* y_, int number_motors) {
  double slidingtime = min(4.0, static_cast<double>(reinforce_interval) / 2);
  fillSensorBuffer(x_, number_sensors);
  if (t > buffersize) {
    if (t % managementInterval == 0) {
      management();
    }
    reward += calcReinforcement() / static_cast<double>(reinforce_interval);
    if ((t % reinforce_interval) == 0) {
      conf.qlearning->learn(state, action, reward, 1);
      state = calcState();
      oldreward = reward;
      reward = 0;
      if (!manualControl) { // select a new action
        oldaction = action;
        action = conf.qlearning->select(state);
        // action = conf.qlearning->select_sample(state);
        // newaction = conf.qlearning->select_keepold(state);
      }
    }

    const Matrix& y = calcMotor(action);
    assert((static_cast<int>(y.getM())) == number_motors);
    int ts = t % reinforce_interval;
    if (ts < slidingtime && action != oldaction) {
      // mixture of old and new actions
      const Matrix& y_o = calcMotor(oldaction);
      // store the values into y_ array
      (y_o * (1 - (ts / slidingtime)) + y * (ts / slidingtime)).convertToBuffer(y_, number_motors);
    } else {
      y.convertToBuffer(y_, number_motors); // store the values into y_ array
    }
  } else {
    memset(y_, 0, sizeof(motor) * number_motors);
  }
  fillMotorBuffer(y_, number_motors); // store the plain c-array __PLACEHOLDER_10__ into the y buffer

  // update step counter
  ++t;
};

/// performs one step without learning. Calulates motor commands from sensor inputs.
void
ClassicReinforce::stepNoLearning(const sensor* x, int number_sensors, motor* y, int number_motors) {
  fillSensorBuffer(x, number_sensors);
  memset(y, 0, sizeof(motor) * number_motors); // fixme
  fillMotorBuffer(y, number_motors);
  // update step counter
  ++t;
};

void
ClassicReinforce::fillSensorBuffer(const sensor* x_, int number_sensors) {
  assert(static_cast<unsigned>(number_sensors) == this->number_sensors);
  Matrix x(number_sensors - 0 /* conf.numContext */, 1, x_);
  Matrix x_c(0 /* conf.numContext */, 1, x_ + number_sensors - 0 /* conf.numContext */);
  // put new input vector in ring buffer x_buffer
  putInBuffer(x_buffer.get(), x);
  putInBuffer(x_context_buffer.get(), x_c);
}

void
ClassicReinforce::fillMotorBuffer(const motor* y_, int number_motors) {
  assert(static_cast<unsigned>(number_motors) == this->number_motors);
  Matrix y(number_motors, 1, y_);
  // put new output vector in ring buffer y_buffer
  putInBuffer(y_buffer.get(), y);
}

void
ClassicReinforce::setManualControl(bool mControl, int action_) {
  if (mControl) {
    action = clip(action_, 0, getActionNumber() - 1);
    oldaction = action;
  }
  manualControl = mControl;
}

void
ClassicReinforce::management() {}

void
ClassicReinforce::notifyOnChange(const paramkey& key) {
  if (key == "mancontrol") {
    setManualControl(manualControl);
  } else if (key == "action") {
    setManualControl(manualControl, action);
  } else if (key == "interval") {
    reinforce_interval = max(reinforce_interval, 1);
  }
}

bool
ClassicReinforce::store(FILE* f) const {
  fprintf(f, "%i\n", 0 /* conf.numContext */);

  // save config and controller
  Configurable::print(f, 0);
  conf.qlearning->store(f);
  return true;
}

bool
ClassicReinforce::restore(FILE* f) {
  if (!initialised)
    init(2, 2);

  char buffer[128];
  // we need to use fgets in order to avoid spurious effects with following matrix (binary)
  if ((fgets(buffer, 128, f)) == nullptr)
    return false;
  // conf.numContext = atoi(buffer);

  // save config and controller
  Configurable::parse(f);
  conf.qlearning->restore(f);
  t = 0; // set time to zero to ensure proper filling of buffers
  action = 0;
  oldaction = 0;
  state = 0;
  reward = 0;
  return true;
}

list<Inspectable::iparamkey>
ClassicReinforce::getInternalParamNames() const {
  list<iparamkey> keylist;

  keylist += storeVectorFieldNames(x_context_buffer[0], "XC");
  keylist += string("action");
  keylist += string("state");
  keylist += string("reward");
  keylist += string("coll_rew");
  return keylist;
}

list<Inspectable::iparamval>
ClassicReinforce::getInternalParams() const {
  list<iparamval> l;
  l += x_context_buffer[t % buffersize].convertToList();
  l += static_cast<double>(action);
  l += static_cast<double>(state);
  l += static_cast<double>(oldreward);
  l += conf.qlearning->getCollectedReward();
  return l;
}

list<Inspectable::ILayer>
ClassicReinforce::getStructuralLayers() const {
  list<Inspectable::ILayer> l;
  return l;
}

list<Inspectable::IConnection>
ClassicReinforce::getStructuralConnections() const {
  list<Inspectable::IConnection> l;
  return l;
}
