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

#include "schlangeservo.h"
using namespace std;

namespace lpzrobots {

SchlangeServo::SchlangeServo ( const OdeHandle& odeHandle, const OsgHandle& osgHandle,
                               const SchlangeConf& conf, const std::string& name,
                               const std::string& revision)
  : Schlange(odeHandle, osgHandle, conf, name,
             revision.empty() ? "$Id$" : revision)
{
  addParameter("velocity", &this->conf.velocity);
}

SchlangeServo::~SchlangeServo() { }


/**
 *Reads the actual motor commands from an array, an sets all motors (forces) of the snake to this values.
 *It is an linear allocation.
 *@param motors pointer to the array, motor values are scaled to [-1,1]
 *@param motornumber length of the motor array
 **/
void SchlangeServo::setMotorsIntern( const double* motors, int motornumber )
{
  assert(created);
  int len = min(motornumber, static_cast<int>(servos.size()));
  // controller output as torques
  for (int i = 0; i < len; ++i) {
    servos[i]->set(motors[i]);
  }
}

/**
 *Writes the sensor values to an array in the memory.
 *@param sensors pointer to the arrays

 *@param sensornumber length of the sensor array
 *@return number of actually written sensors
 **/
int SchlangeServo::getSensorsIntern( sensor* sensors, int sensornumber )
{
  assert(created);
  int len = min(sensornumber, getSensorNumberIntern());

  for (int n = 0; n < len; ++n)  {
    sensors[n] = servos[n]->get();
  }

  return len;
}


/** creates vehicle at desired position
    @param pos struct Position with desired position
*/
void SchlangeServo::create(const osg::Matrix& pose){
  Schlange::create(pose);

  //*****************joint definition***********
  for ( int n = 0; n < conf.segmNumber-1; ++n )  {

    HingeJoint* j = new HingeJoint(objects[n], objects[n+1],
                                   (objects[n]->getPosition() + objects[n+1]->getPosition())/2,
                                   Axis(0,0,1)* pose);
    j->init(odeHandle, osgHandle, true, conf.segmDia * 1.02);


    // making stops bouncy
    //    j->setParam (dParamBounce, 0.9 );
      //    j->setParam (dParamBounce2, 0.9 ); // universal

    joints.push_back(j);

    HingeServo* servo;
    if(conf.useServoVel){
      servo =  new OneAxisServoVel(odeHandle, j, -conf.jointLimit, conf.jointLimit,
                                   conf.motorPower, conf.frictionJoint, conf.velocity);
    }else{
      servo =  new HingeServo(j, -conf.jointLimit, conf.jointLimit,
                              conf.motorPower,conf.frictionJoint,0,100);
    }
    servos.push_back(servo);
    notifyOnChange("dummy");
    //    frictionmotors.push_back(new AngularMotor1Axis(odeHandle, j, conf.frictionJoint) );
  }
}

void SchlangeServo::notifyOnChange(const paramkey& key){

  for (vector<HingeServo*>::iterator i = servos.begin(); i!= servos.end(); ++i) {
    if(*i) (*i)->setDamping(conf.frictionJoint);
  }
  for (vector<HingeServo*>::iterator i = servos.begin(); i!= servos.end(); ++i) {
    if(*i) (*i)->setPower(conf.motorPower);
  }
  for (vector<HingeServo*>::iterator i = servos.begin(); i!= servos.end(); ++i) {
    if(*i) (*i)->setMaxVel(conf.velocity);
  }
}


/** destroys vehicle and space
 */
void SchlangeServo::destroy(){
  if (created){
    Schlange::destroy();
    for (vector<HingeServo*>::iterator i = servos.begin(); i!= servos.end(); ++i) {
      if(*i) delete *i;
    }
    servos.clear();
  }
}

}
