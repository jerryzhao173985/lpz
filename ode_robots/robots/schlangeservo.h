/************************************************************************/
/*schlangeservo.h                                                        */
/*Snake with PID Servo motors (just one motor per joint)                     */
/*@author Georg Martius                                                 */
/*                                                                        */
/************************************************************************/
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
#ifndef __SCHLANGESERVO_H
#define __SCHLANGESERVO_H


#include "schlange.h"
#include "oneaxisservo.h"

namespace lpzrobots {

  /**
   * This is a class, which models a snake like robot.
   * It consists of a number of equal elements, each linked
   * by a joint hinge powered by 1 servos
   **/
  class SchlangeServo : public Schlange {
  private:
    std::vector <HingeServo*> servos;

  public:
    SchlangeServo ( const OdeHandle& odeHandle, const OsgHandle& osgHandle,
                    const SchlangeConf& conf,
                    const std::string& name, const std::string& revision="");


    virtual ~SchlangeServo() override;

    /**
     *Reads the actual motor commands from an array,
     *an sets all motors of the snake to this values.
     *It is an linear allocation.
     *@param motors pointer to the array, motor values are scaled to [-1,1]
     *@param motornumber length of the motor array
     **/
    virtual void setMotorsIntern( const double* motors, int motornumber ) override;

    /**
     *Writes the sensor values to an array in the memory.
     *@param sensors pointer to the array
     *@param sensornumber length of the sensor array
     *@return number of actually written sensors
     **/
    virtual int getSensorsIntern( double* sensors, int sensornumber ) override;

    /** returns number of sensors
     */
    virtual int getSensorNumberIntern() const override { assert(created); return servos.size(); }

    /** returns number of motors
     */
    virtual int getMotorNumberIntern() const override { assert(created); return servos.size(); }

    /******** CONFIGURABLE ***********/
    virtual void notifyOnChange(const paramkey& key) override;

  private:
    virtual void create(const osg::Matrix& pose) override;
    virtual void destroy() override;
  };

}

/*
Parameters for invertmotornstep controller:
motorpower=5
sensorfactor=5

controller->setParam(__PLACEHOLDER_3__, 0.000);
controller->setParam(__PLACEHOLDER_4__, 0.01);
controller->setParam(__PLACEHOLDER_5__, 0.01);
controller->setParam(__PLACEHOLDER_6__, 0.0001);
controller->setParam(__PLACEHOLDER_7__, 1);
controller->setParam(__PLACEHOLDER_8__, 1);
controller->setParam(__PLACEHOLDER_9__, 1);
controller->setParam(__PLACEHOLDER_10__, 5);
global.odeConfig.setParam(__PLACEHOLDER_11__,4);


High frequency modes and then low-dim modes occur with
controller->setParam(__PLACEHOLDER_12__, 0.0001);
controller->setParam(__PLACEHOLDER_13__, 0.0007);
controller->setParam(__PLACEHOLDER_14__, 0.01);
controller->setParam(__PLACEHOLDER_15__, 0.01);
controller->setParam(__PLACEHOLDER_16__, 0.0001);
controller->setParam(__PLACEHOLDER_17__, 1);
controller->setParam(__PLACEHOLDER_18__, 1);
controller->setParam(__PLACEHOLDER_19__, 1);
controller->setParam(__PLACEHOLDER_20__, 1);
global.odeConfig.setParam(__PLACEHOLDER_21__,1);


*/


#endif

