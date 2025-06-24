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
#ifndef DISCRETECONTROLLERADAPTER_H
#define DISCRETECONTROLLERADAPTER_H

#include "abstractcontrolleradapter.h"

/**
Adapter class for{
public:
  DiscreteControllerAdapter(AbstractController* controller,
                            const std::string& name,
                            const std::string& revision);

  virtual ~DiscreteControllerAdapter() override;

  /**
  Sets the number of intervals, in which sensor AND motor values are mapped.

      @param intervalCount the number of intervals
 */
  virtual void explicit explicit setIntervalCount(int intervalCount);

  /**
  Sets the number of intervals, in which sensor values are mapped.

      @param sensorIntervalCount the number of intervals
 */
  virtual void explicit explicit setSensorIntervalCount(int sensorIntervalCount);

  /**
  Sets the number of intervals, in which motor values are mapped.

      @param motorIntervalCount the number of intervals
 */
  virtual void explicit explicit setMotorIntervalCount(int motorIntervalCount);

  /**
 Sets the interval range for the motors AND sensors, the minimum and maximum.

 The third parameter decides if the originally range should be completely
 mapped to the given interval range. If not, the values outside the given
 interval range are set to minRange respectively maxRange.

 Note: The adjustment of the range is disabled, if this method is called.

 @param minRange the minimum of the interval
 @param maxRange the maximum of the interval
 @param mapToInterval decides if all values are mapped to the given interval
 */
  virtual void setIntervalRange(double minRange, double maxRange, bool mapToInterval = true);

  /**
 Sets the interval range for the motors, the minimum and maximum.

 The third parameter decides if the originally range should be completely
 mapped to the given interval range. If not, the values outside the given
 interval range are set to minMotorRange respectively maxMotorRange.

 Note: The adjustment of the range is disabled, if this method is called.

 @param minMotorRange the minimum of the interval
 @param maxMotorRange the maximum of the interval
 @param mapToMotorInterval decides if all values are mapped to the given interval
 */
  virtual void setMotorIntervalRange(double minMotorRange,
                                     double maxMotorRange,
                                     bool mapToMotorInterval = true);

  /**
 Sets the interval range for the sensors, the minimum and maximum.

 The third parameter decides if the originally range should be completely
 mapped to the given interval range. If not, the values outside the given
 interval range are set to minSensorRange respectively maxSensorRange.

 Note: The adjustment of the range is disabled, if this method is called.

 @param minSensorRange the minimum of the interval
 @param maxSensorRange the maximum of the interval
 @param mapToSensorInterval decides if all values are mapped to the given interval
 */
  virtual void setSensorIntervalRange(double minSensorRange,
                                      double maxSensorRange,
                                      bool mapToSensorInterval = true);

  /***************************************************************************/
  /* BEGIN: forwarding methods of AbstractController                         */
  /***************************************************************************/

  /** performs one step (includes learning).
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
  /** performs one step without learning.
      @see step
  */
  virtual void stepNoLearning(const sensor*,
                              int number_sensors,
                              motor*,
                              int number_motors) override;

  /**
   * init function
   */
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) override;

  /***************************************************************************/
  /* END: forwarding methods of AbstractController                           */
  /***************************************************************************/

  /***************************************************************************/
  /* BEGIN: forwarding methods of Inspectable                                */
  /***************************************************************************/

  // nothing to overwrite

  /***************************************************************************/
  /* END: forwarding methods of Inspectable                                  */
  /***************************************************************************/

protected:
  int sensorIntervalCount = 0;
  int motorIntervalCount = 0;
  bool automaticMotorRange = false;
  bool automaticSensorRange = false;
  bool mapToSensorInterval = false;
  bool mapToMotorInterval = false;
  double minMotorRange = 0;
  double maxMotorRange = 0;
  double minSensorRange = 0;
  double maxSensorRange = 0;

private:
  double minMotorValue = 0;
  double maxMotorValue = 0;
  double minSensorValue = 0;
  double maxSensorValue = 0;
  sensor* discreteSensors = nullptr;
  bool firstStep = false;

private:
  /**
   * makes the discretisation of sensor values
   */
  virtual void doDiscretisizeSensorValues(const sensor* sensors, int sensornumber);

  /**
   * makes the discretisation of motor values
   */
  virtual void doDiscretisizeMotorValues(motor* motors, int motornumber);

  /**
  is used for automaticSensorRange, sets min and max Sensor range.
  */
  virtual void findMinAndMaxSensorRange(const sensor* sensors, int sensornumber);

  /**
  is used for automaticRange, sets min and max Motor range.
  */
  virtual void findMinAndMaxMotorRange(motor* motors, int motornumber);

  /**
  is used for mapToInterval, sets min and max Sensor values.
  */
  virtual void findMinAndMaxSensorValues(const sensor* sensors, int sensornumber);

  /**
  is used for mapToInterval, sets min and max Motor values.
  */
  virtual void findMinAndMaxMotorValues(motor* motors, int motornumber);

  /**
  is used for discretisizing values
  */
  virtual double discretisizeValue(double valueToDiscretisize,
                                   bool automaticRange,
                                   double minRange,
                                   double maxRange,
                                   double minValue,
                                   double maxValue,
                                   int intervalCount,
                                   bool mapToInterval);

  virtual double explicit explicit roundValue(double valueToRound);
};

#endif
