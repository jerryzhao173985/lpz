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
#ifndef __MEASUREADAPTER_H
#define __MEASUREADAPTER_H

#include "abstractcontrolleradapter.h"
#include "backcaller.h"
#include "complexmeasure.h"
#include "statistictools.h"
#include <memory>
#include <vector>

// begin forward declaration
// end forward declaration

/**
 * This is a passive controller adapter who is passive and can handle AbstractMeasures.
 * Normally the sensor and/or motor values are measured.
 * @see AbstractControllerAdapter
 */
class MeasureAdapter : public AbstractControllerAdapter {

public:
  /**
   * Constructs the MeasureAdapter.
   *
   */
  explicit MeasureAdapter(AbstractController* controller,
                          const std::string& name = "MeasureAdapter",
                          const std::string& revision = "$ID$");

  virtual ~MeasureAdapter() override;

  /**
   * Adds a ComplexMeasure for measuring sensor values. For each
   * sensor a ComplexMeasure is created.
   */
  virtual std::list<ComplexMeasure*> addSensorComplexMeasure(const char* measureName,
                                                             ComplexMeasureMode mode,
                                                             int numberBins,
                                                             int stepSize);

  /****************************************************************************/
  /*        BEGIN methods of AbstractController                                 */
  /****************************************************************************/

  /** initialisation of the controller with the given sensor/ motornumber
   * Must NORMALLY be called before use. For all ControllerAdapters
   * call first AbstractControllerAdapter::init(sensornumber,motornumber)
   * if you overwrite this method
   */
  virtual void init(int sensornumber, int motornumber, RandGen* randGen = 0) override;

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
  virtual void stepNoLearning(const sensor* sensors,
                              int sensornumber,
                              motor* motors,
                              int motornumber) override;

  /****************************************************************************/
  /*        END methods of AbstractController                                             */
  /****************************************************************************/

  /****************************************************************************/
  /*        BEGIN methods of Storeable                                                   */
  /****************************************************************************/

  /****************************************************************************/
  /*        END methods of Storeable                                                      */
  /****************************************************************************/

  /****************************************************************************/
  /*        BEGIN methods of Inspectable                                                  */
  /****************************************************************************/

  // nothing to overwrite

  /****************************************************************************/
  /*        END methods of Inspectable                                                   */
  /****************************************************************************/

  [[nodiscard]] virtual StatisticTools* getStatisticTools() const {
    return st.get();
  }

protected:
  std::unique_ptr<StatisticTools> st;
  bool initialized = false;
  std::vector<motor> motorValues;
  std::vector<sensor> sensorValues;

  // Rule of 5: Delete copy operations, allow move
  MeasureAdapter(const MeasureAdapter&) = delete;
  MeasureAdapter& operator=(const MeasureAdapter&) = delete;
  MeasureAdapter(MeasureAdapter&&) = default;
  MeasureAdapter& operator=(MeasureAdapter&&) = default;
};

#endif
