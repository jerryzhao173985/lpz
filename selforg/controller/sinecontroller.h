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
#ifndef __SINECONTROLLER_H
#define __SINECONTROLLER_H

#include "abstractcontroller.h"
#include <cstdio>
#include <memory>

/**
 * Sine controller class that generates sine, sawtooth or impulse patterns on motor outputs.
 * Does NOT use the sensors.
 */
class SineController : public AbstractController {
public:
  enum class function { Sine, SawTooth, Impulse };

  /**
     @param controlmask bitmask to select channels to control (default all)
     @param function controller function to use
   */
  SineController(unsigned long int controlmask = (~0), function func = function::Sine);

  /** initialisation of the controller with the given sensor/ motornumber
      Must be called before use.
  */
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override;

  /** @return Number of sensors the controller was initialised
      with or 0 if not initialised */
  virtual int getSensorNumber() const override {
    return number_sensors;
  }

  /** @return Number of motors the controller was initialised
      with or 0 if not initialised */
  virtual int getMotorNumber() const override {
    return number_motors;
  }

  /** performs one step ( the same as StepNoLearning).
      Calculates motor commands from sensor inputs.
      @param sensors sensors inputs scaled to [-1,1]
      @param sensornumber length of the sensor array
      @param motors motors outputs. MUST have enough space for motor values!
      @param motornumber length of the provided motor array
  */
  virtual void step(const sensor* sensors,
                    int sensornumber,
                    motor* motors,
                    int motornumber) override;
  /** performs one step.
      @see step
  */
  virtual void stepNoLearning(const sensor*,
                              int number_sensors,
                              motor*,
                              int number_motors) override;

  /********* STORABLE INTERFACE ******/
  /// @see Storable
  virtual bool store(FILE* f) const {
    Configurable::print(f, "");
    return true;
  }

  /// @see Storable
  virtual bool restore(FILE* f) {
    Configurable::parse(f);
    return true;
  }

  /// sine
  static double sine(double x, double _unused);
  /// saw tooth shape oscillator
  static double sawtooth(double x, double _unused);
  /// impuls shaped oscillator (+-1 for impulsWidth part of the time)
  static double impuls(double x, double impulsWidth);

protected:
  std::string name;
  int number_sensors = 0;
  int number_motors = 0;
  unsigned long int controlmask; // bitmask to select channels. (the others are set to 0)
  bool individual = false;

  AbstractController::paramval period;
  AbstractController::paramval phaseShift;
  AbstractController::paramval impulsWidth;
  double phase = 0; // phase of oscillator
  AbstractController::paramval amplitude;

  double (*osci)(double x, double param); // oscillator function
};

class MultiSineController : public SineController {
public:
  MultiSineController(unsigned long int controlmask = (~0),
                               function func = function::Sine);
  virtual ~MultiSineController() override;
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) override;
  virtual void stepNoLearning(const sensor*,
                              int number_sensors,
                              motor*,
                              int number_motors) override;

protected:
  std::unique_ptr<double[]> periods;
  std::unique_ptr<double[]> phaseShifts;
  std::unique_ptr<double[]> amplitudes;
  std::unique_ptr<double[]> offsets;
  long t = 0;
};

#endif
