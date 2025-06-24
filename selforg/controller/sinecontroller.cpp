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

#include "sinecontroller.h"
#include "controller_misc.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>

// Removed using namespace std for better practice

SineController::SineController(unsigned long int controlmask, function func)
  : AbstractController("sinecontroller", "1.0")
  , controlmask(controlmask)
  , individual(false) {
  phase = 0;
  addParameterDef("period", &period, 50);
  addParameterDef("phaseshift", &phaseShift, 1);
  if (func == function::Impulse)
    addParameterDef("impulswidth", &impulsWidth, 0.5);
  addParameterDef("amplitude", &amplitude, 1);
  switch (func) {
    case function::Sine:
      osci = sine;
      break;
    case function::SawTooth:
      osci = sawtooth;
      break;
    case function::Impulse:
      osci = impuls;
      break;
    default:
      assert("Unknown function type");
  }

  number_sensors = 0;
  number_motors = 0;
};

/** initialisation of the controller with the given sensor/ motornumber
    Must be called before use.
*/
void
SineController::init(int sensornumber, int motornumber, RandGen* randGen) {
  number_sensors = sensornumber;
  number_motors = motornumber;
};

void
SineController::step(const sensor* sensors, int sensornumber, motor* motors, int motornumber) {
  stepNoLearning(sensors, sensornumber, motors, motornumber);
};

void
SineController::stepNoLearning(const sensor* sensors,
                               int number_sensors,
                               motor* motors,
                               int number_motors) {

  for (int i = 0; i < std::min(number_motors, static_cast<int>(sizeof(controlmask) * 8)); ++i) {
    if ((controlmask & (1 << i)) != 0) {
      motors[i] = amplitude * osci(phase + i * phaseShift * M_PI / 2, impulsWidth);
    } else {
      motors[i] = 0;
    }
  }
  if (period != 0) {
    phase += 2 * M_PI / period;
    if (phase > 2 * M_PI)
      phase -= 2 * M_PI;
  }
};

double
SineController::sine(double x, double _unused) {
  return sin(x);
}

double
SineController::sawtooth(double x, double _unused) {
  while (x > M_PI)
    x -= 2 * M_PI;
  while (x < -M_PI)
    x += 2 * M_PI;
  // x is centered around -PI and PI.
  if (x > -M_PI / 2 && x <= M_PI / 2)
    return x / M_PI;
  else {
    if (x < 0)
      return -(x + M_PI) / M_PI;
    else
      return -(x - M_PI) / M_PI;
  }
}

double
SineController::impuls(double x, double impulsWidth) {
  while (x > M_PI)
    x -= 2 * M_PI;
  while (x < -M_PI)
    x += 2 * M_PI;
  // x is centered around -PI and PI.
  // +-1 for |x| in ((0.5-impulswidth/2)*M_PI,(0.5+impulswidth/2)*M_PI]
  if (fabs(x) > (0.5 - impulsWidth / 2) * M_PI && fabs(x) < (0.5 + impulsWidth / 2) * M_PI)
    return sign(x);
  else
    return 0;
}

MultiSineController::MultiSineController(unsigned long int controlmask, function func)
  : SineController(controlmask, func)
  , t(0) {
  Configurable::setName("multisinecontroller");
  Inspectable::setNameOfInspectable("multisinecontroller");
};

MultiSineController::~MultiSineController() {
  // Smart pointers automatically clean up
}

/** initialisation of the controller with the given sensor/ motornumber
    Must be called before use.
*/
void
MultiSineController::init(int sensornumber, int motornumber, RandGen* randGen) {
  SineController::init(sensornumber, motornumber, randGen);
  periods = std::make_unique<double[]>(motornumber);
  phaseShifts = std::make_unique<double[]>(motornumber);
  amplitudes = std::make_unique<double[]>(motornumber);
  offsets = std::make_unique<double[]>(motornumber);
  for (int i = 0; i < std::min(number_motors, static_cast<int>(sizeof(controlmask) * 8)); ++i) {
    if ((controlmask & (1 << i)) != 0) {
      addParameterDef("period" + std::itos(i), &periods[i], period);
      addParameterDef("phaseshift" + std::itos(i), &phaseShifts[i], phaseShift * i);
      addParameterDef("amplitude" + std::itos(i), &amplitudes[i], amplitude);
      addParameterDef("offset" + std::itos(i), &offsets[i], 0);
    }
  }
};

void
MultiSineController::stepNoLearning(const sensor* sensors,
                                    int number_sensors,
                                    motor* motors,
                                    int number_motors) {

  for (int i = 0;
       i < std::min(number_motors, static_cast<int>(sizeof(SineController::controlmask) * 8));
       ++i) {
    if ((SineController::controlmask & (1 << i)) != 0 && periods[i] != 0) {
      motors[i] =
        amplitudes[i] * SineController::osci(t * 2 * M_PI / periods[i] + phaseShifts[i] * M_PI / 2,
                                             SineController::impulsWidth) +
        offsets[i];
    } else {
      motors[i] = 0;
    }
  }
  ++t;
};
