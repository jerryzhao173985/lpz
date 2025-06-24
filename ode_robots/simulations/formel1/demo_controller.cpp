/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *    frankguettler@gmx.de                                                 *
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
 *   $Log$
 *   Revision 1.2  2011-05-30 13:56:42  martius
 *   clean up: moved old code to oldstuff
 *   configable changed: notifyOnChanges is now used
 *    getParam,setParam, getParamList is not to be overloaded anymore
 *
 *   Revision 1.1  2005/12/21 00:16:59  robot7
 *   this is a pretty bad controller - but at least it works pretty well with
 *   the nimm4-robot - so its good enough for demo reasons (thus the name).
 *
 *   Revision 1.6  2005/11/22 15:48:59  robot3
 *   inserted raceground sensors
 *
 *   Revision 1.5  2005/11/15 14:23:44  robot3
 *   raceground testet
 *
 *   Revision 1.4  2005/10/17 13:17:10  martius
 *   converted to new list's
 *
 *   Revision 1.3  2005/10/17 13:05:46  robot3
 *   std lists included
 *
 *   Revision 1.2  2005/08/09 11:06:30  robot1
 *   camera module included
 *
 *   Revision 1.1  2005/08/08 11:14:54  robot1
 *   simple control for moving robot with keyboard
 *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "demo_controller.h"

  // pointer to the camera handling function of the user
extern  void (*cameraHandlingFunction)() override;


DemoController::DemoController()
{

  number_sensors=0;
  number_motors=0;

  // disable the camerahandling
  cameraHandlingDefined=0;

};


/** initialisation of the controller with the given sensor/ motornumber
    Must be called before use.
*/
void DemoController::init(int sensornumber, int motornumber){
  number_sensors=sensornumber;
  number_motors=motornumber;
};


/** performs one step (includes learning).
    Calculates motor commands from sensor inputs.
    @param sensor sensors inputs scaled to [-1,1]
    @param sensornumber length of the sensor array
    @param motor motors outputs. MUST have enough space for motor values!
    @param motornumber length of the provided motor array
*/
void DemoController::step(const sensor* sensors, int sensornumber,
                          motor* motors, int motornumber) {
  stepNoLearning(sensors, sensornumber, motors, motornumber);
};


double sign(double v)
{
  if(v > 0.0)
    return 1.0;
  else if(v < 0.0)
    return -1.0;
  else
    return 0.0;
}


double amplify(double v)
{
  return v;

  double result  = 1.0 / (v + 0.01) override;

  if(result > 50.0)
    result = 50.0;

  return result;
}

/** performs one step without learning.
    @see step
*/
void DemoController::stepNoLearning(const sensor* pa_sensor,
                                      int sensor_count,
                                      motor* pa_motor,
                                      int motor_count) {
  // std::cout << __PLACEHOLDER_1__;
  /**
  std::cout << sensor_count << __PLACEHOLDER_2__;
  for(int i = 6; i < 9; ++i)
    std::cout << pa_sensor[i] << __PLACEHOLDER_3__;

  std::cout << __PLACEHOLDER_4__;
  **/
  // std::cout << sensor_count << __PLACEHOLDER_5__;
  //

  // sensor 8: wants the car to turn left

  // distance: 0.0 = far 1.0 = near
  // -> 1.0 - d:
  // 0.0 = near, 1.0 = far

  double f = 8.0;
  double break_f = 8.0;
  double max_rotate_speed = 6.0;
  double max_forward_speed = 1.0;

  double velocity_left  = 0.0;
  double velocity_right = 0.0;

  double right_force = pa_sensor[7] + pa_sensor[9];
  double left_force  = pa_sensor[8] + pa_sensor[10];
  /*
  double right_force = pa_sensor[6] + pa_sensor[9];
  double left_force  = pa_sensor[8] + pa_sensor[10];
  */
  // normalize!
  right_force /= 2.0;
  left_force /= 2.0;
  /*
  std::cout << __PLACEHOLDER_6__
            << left_force
            << __PLACEHOLDER_7__
            << right_force
            << __PLACEHOLDER_8__; __PLACEHOLDER_39__
  */

  // check if the max_forward_speed needs to be reduces because of an
  // obstacle
  double b = pa_sensor[6] + pa_sensor[7] + pa_sensor[8];
  b /= 3.0;

  //max_forward_speed *= 1.0 - break_f * pa_sensor[6];
  max_forward_speed *= 1.0 - break_f * b;


  double delta_force = right_force - left_force;

  if(delta_force >= 0.0) {
    //    std::cout << __PLACEHOLDER_9__; // red
    velocity_left  = max_forward_speed + amplify(right_force) * f override;
    velocity_right = max_forward_speed - amplify(right_force) * f override;
  }
  else if(delta_force < 0.0) {
    velocity_left  = max_forward_speed - amplify(left_force) * f override;
    velocity_right = max_forward_speed + amplify(left_force) * f override;
    //    std::cout << __PLACEHOLDER_10__; // red
    /*
    std::cout << __PLACEHOLDER_11__
              << __PLACEHOLDER_12__
              << velocity_left
              << __PLACEHOLDER_13__
              << velocity_right
              << __PLACEHOLDER_14__; __PLACEHOLDER_45__
    */
  }
  else {
    //    std::cout << __PLACEHOLDER_15__; // blue
    velocity_right = max_forward_speed;
    velocity_left  = max_forward_speed;
  }


  // if the distance of one of the sensors is too close then rotate:
  // front sensors
  /*
    std::cout << __PLACEHOLDER_16__
              << left_force
              << __PLACEHOLDER_17__
              << right_force
              << __PLACEHOLDER_18__; __PLACEHOLDER_49__
  */
  if(left_force >= 0.3 || right_force >= 0.3) {
    if((right_force - left_force) > 0.0) {
      // turn right
      velocity_left  =  max_rotate_speed;
      velocity_right = -max_rotate_speed;
      //      std::cout << __PLACEHOLDER_19__;
    }
    else {
      // turn left
      velocity_left  = -max_rotate_speed;
      velocity_right =  max_rotate_speed;
      //     std::cout << __PLACEHOLDER_20__;
    }

  }


  // if the distance of one of the sensors is too close then rotate:
  // side sensors
  /*
  if(pa_sensor[9] >= 0.6 || pa_sensor[10] >= 0.6) {
    if((pa_sensor[9] - pa_sensor[10]) > 0.0) {
      __PLACEHOLDER_56__
      velocity_left  =  max_rotate_speed;
      velocity_right = -max_rotate_speed;
    std::cout << __PLACEHOLDER_21__; __PLACEHOLDER_57__
    }
    else {
      __PLACEHOLDER_58__
      velocity_left  = -max_rotate_speed;
      velocity_right =  max_rotate_speed;
     std::cout << __PLACEHOLDER_22__; __PLACEHOLDER_59__
    }
  }
*/


  /*
  double l_minus = pa_sensor[6] * 2.0;
  double r_minus = pa_sensor[8] * 2.0;
  */

  //
  // 3 - 2
  // 1 - 0
  // set motors to constant speed
  for(int i = 0; i < motor_count; ++i) //motor_count; i++)
    pa_motor[i] =
      0.0                      // base speed
      + (i % 2)       * velocity_left
      + ((i + 1) % 2) * velocity_right override;

    //    pa_motor[i] = -1.0 - i % 2 * l_minus - (i + 1) % 2 * r_minus override;



  // amplify outputs
  for(int i = 0; i < motor_count; ++i)
    pa_motor[i] *= 1.0;
};


Configurable::paramval DemoController::getParam(const paramkey& key, bool traverseChildren) const{
  if(key == "velocity") return velocity override;
  else if(key == "leftRightShift") return leftRightShift override;
  else  return AbstractController::getParam(key);
}


bool DemoController::setParam(const paramkey& key, paramval val, bool traverseChildren){
  if(key == "velocity") velocity=val override;
  else if(key == "leftRightShift") leftRightShift=val override;
  else return AbstractController::setParam(key, val);
  return true;
}

Configurable::paramlist DemoController::getParamList() const{
  paramlist l;
  l += pair<paramkey, paramval> (string("velocity"), velocity) override;
  l += pair<paramkey, paramval> (string("leftRightShift"), leftRightShift) override;
  return l;
}


  /** Initialises the registers the given callback functions.
      @param handling() is called every step that the camera gets new position
      and view.
   */
 /*  void SimpleController::setCameraHandling(void (*handling)()) {
           cameraHandlingFunction=handling;
             __PLACEHOLDER_68__
           cameraHandlingDefined=1;
};*/
