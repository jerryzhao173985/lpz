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
#include "measureadapter.h"
#include <sstream>
#include <algorithm>
#include <iostream>

MeasureAdapter::MeasureAdapter(AbstractController* controller,
                               const std::string& name,
                               const std::string& revision)
  : AbstractControllerAdapter(controller, name, revision)
  , st(std::make_unique<StatisticTools>("MeasureAdapter's ST"))
  , initialized(false)
  , motorValues()
  , sensorValues() {
  // Note: StatisticTools does not inherit from Callbackable
  // addCallbackable(st.get());
  addInspectable(st.get());
}

MeasureAdapter::~MeasureAdapter() {
  // Automatic cleanup by unique_ptr
}

std::list<ComplexMeasure*>
MeasureAdapter::addSensorComplexMeasure(const char* measureName,
                                        ComplexMeasureMode mode,
                                        int numberBins,
                                        int stepSize) {
  std::list<ComplexMeasure*> cmlist;
  if (initialized) {
    for (int i = 0; i < controller->getSensorNumber(); ++i) {
      std::stringstream name;
      name << measureName << " [" << i << "]";
      ComplexMeasure* cm = new ComplexMeasure(name.str().c_str(), mode, numberBins);
      switch (mode) {
        case ENT:
          cm->addObservable(sensorValues.data()[i], -1.0, 1.0);
          cm->setStepSize(stepSize);
          break;
        case ENTSLOW:
          cm->addObservable(sensorValues.data()[i], -1.0, 1.0);
          break;
        default:
          break;
      }
      st->addMeasure(cm);
      cmlist.push_back(cm);
    }
  } else {
    std::cerr << "ERROR: The method" << std::endl
              << "       addSensorComplexMeasure(const char* measureName, ComplexMeasureMode mode,int "
                 "numberBins, int stepSize)"
              << std::endl
              << "must be called after the initialization of the Agent!" << std::endl;
    // Return empty list instead of crashing
    return cmlist;
  }
  return cmlist;
}

/****************************************************************************/
/*        BEGIN methods of AbstractController                                 */
/****************************************************************************/

void
MeasureAdapter::init(const int sensornumber, const int motornumber, RandGen* randGen) {
  // call the same method of super class AbstractControllerAdapter
  AbstractControllerAdapter::init(sensornumber, motornumber, randGen);
  // Initialize sensor and motor value arrays
  sensorValues.resize(sensornumber);
  motorValues.resize(motornumber);
  initialized = true;
}

void
MeasureAdapter::step(const sensor* sensors, int sensornumber, motor* motors, int motornumber) {
  // call the same method of super class AbstractControllerAdapter
  AbstractControllerAdapter::step(sensors, sensornumber, motors, motornumber);
  // store motor and sensor values in motorValues and sensorValues
  std::copy(motors, motors + motornumber, motorValues.begin());
  std::copy(sensors, sensors + sensornumber, sensorValues.begin());
  callBack();
}

void
MeasureAdapter::stepNoLearning(const sensor* sensors,
                               int sensornumber,
                               motor* motors,
                               int motornumber) {
  AbstractControllerAdapter::stepNoLearning(sensors, sensornumber, motors, motornumber);
  // store motor and sensor values in motorValues and sensorValues
  std::copy(motors, motors + motornumber, motorValues.begin());
  std::copy(sensors, sensors + sensornumber, sensorValues.begin());
  callBack();
}

/****************************************************************************/
/*        END methods of AbstractController                                   */
/****************************************************************************/

/****************************************************************************/
/*        BEGIN methods of Storeable                                                   */
/****************************************************************************/

/****************************************************************************/
/*        END methods of Storeable                                                      */
/****************************************************************************/

/****************************************************************************/
/*        BEGIN methods of Inspectable                                        */
/****************************************************************************/

/****************************************************************************/
/*        END methods of Inspectable                                          */
/****************************************************************************/
