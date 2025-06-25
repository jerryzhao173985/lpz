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
#ifndef ABSTRACTCONTROLLER_H
#define ABSTRACTCONTROLLER_H

#include "configurable.h"
#include "inspectable.h"
#include "randomgenerator.h"
#include "sensormotorinfo.h"
#include "storeable.h"
#include <cstdio>
#include <list>
#include <map>

/**
 * Abstract class for robot controller (with some basic functionality).
 * The controller gets a number of sensor values each timestep
 * and has to generate a number of motor values.
 *
 * Interface assumes the following usage:
 *  - init() is called first to initialise the controller with the given
 *  number of sensors and motors
 *  - step() is called for each timestep with the current sensor values
 *  and returns the motor values to be used
 *  - for special purposes you can use stepNoLearning()
 */
class AbstractController
  : public Configurable
  , public Inspectable {
public:
  using sensor = double;
  using motor = double;

  /// contructor (hint: use $ID$ for revision)
  AbstractController(const std::string& name, const std::string& revision)
    : Configurable(name, revision)
    , Inspectable(name) {}

  /// virtual destructor
  ~AbstractController() = default;

  /** initialisation of the controller with the given sensor/ motornumber
      Must be called before use. The random generator is optional.
  */
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = nullptr) = 0;

  /** @return Number of sensors the controller
      was initialised with or 0 if not initialised */
  [[nodiscard]] virtual int getSensorNumber() const = 0;

  /** @return Number of motors the controller
      was initialised with or 0 if not initialised */
  [[nodiscard]] virtual int getMotorNumber() const = 0;

  /** performs one step (includes learning).
      Calculates motor commands from sensor inputs.
      @param sensors sensors inputs scaled to [-1,1]
      @param sensornumber length of the sensor array
      @param motors motors outputs. MUST have enough space for motor values!
      @param motornumber length of the provided motor array
  */
  virtual void step(const sensor* sensors, int sensornumber, motor* motors, int motornumber) = 0;
  /** performs one step without learning.
      @see step
  */
  virtual void stepNoLearning(const sensor*, int number_sensors, motor*, int number_motors) = 0;

  /** called in motor babbling phase.
      the motor values are given (by babbling controller) and
      this controller can learn the basic relations from observed sensors/motors
   */
  virtual void motorBabblingStep(const sensor* /*unused*/,
                                 int number_sensors,
                                 const motor* /*unused*/,
                                 int number_motors) {};

  /** the controller is notified about the information on sensor.
      This is called after init and before step
      By default the sensorIndexMap and sensorInfoMap is updated and
      can be accessed by SIdx() and SInfo()
  */
  virtual void sensorInfos(const std::list<SensorMotorInfo>& sensorInfos);

  /** the controller is notified about the information on motors.
      This is called after init and before step
      By default the motorIndexMap and motorInfoMap is updated and
      can be accessed by MIdx() and MInfo()

  */
  virtual void motorInfos(const std::list<SensorMotorInfo>& motorInfos);

  /** returns the index of the sensor with the given name
      (if not found then 0 and all sensor names are printed) */
  virtual int SIdx(const std::string& name) const;
  /** returns the index of the motor with the given name
      (if not found then 0 and all motor names are printed) */
  virtual int MIdx(const std::string& name) const;
  /** returns the Information for the sensor with given index */
  virtual SensorMotorInfo SInfo(int index) const;
  /** returns the Information for the motor with given index */
  virtual SensorMotorInfo MInfo(int index) const;

protected:
  std::map<std::string, int> sensorIndexMap{};
  std::map<std::string, int> motorIndexMap{};
  std::map<int, SensorMotorInfo> sensorInfoMap{};
  std::map<int, SensorMotorInfo> motorInfoMap{};
};

#endif
