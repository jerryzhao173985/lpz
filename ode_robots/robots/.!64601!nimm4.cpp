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
#include <cassert>
#include <ode-dbl/ode.h>

// include primitives (box, spheres, cylinders ...)
#include "primitive.h"
#include "osgprimitive.h"

// include joints
#include "joint.h"

// include header file
#include "nimm4.h"

using namespace osg;


namespace lpzrobots {

  // constructor:
  // - give handle for ODE and OSG stuff
  // - size of robot, maximal used force and speed factor are adjustable
  // - sphereWheels switches between spheres or wheels as wheels
  //   (wheels are only drawn, collision handling is always with spheres)
  Nimm4::Nimm4(const OdeHandle& odeHandle, const OsgHandle& osgHandle,
               const std::string& name,
               double size/*=1.0*/, double force /*=3*/, double speed/*=15*/,
               bool sphereWheels /*=true*/)
    : // calling OdeRobots construtor with name of the actual robot
      OdeRobot(odeHandle, osgHandle, name, "$Id$")
  {
    // robot is not created till now
    created=false;

    // choose color (here the color of the __PLACEHOLDER_5__ candy is used,
    // where the name of the Nimm2 and Nimm4 robots comes from ;-)
    this->osgHandle.color = Color(2, 156/255.0, 0, 1.0f);

    // maximal used force is calculated from the force factor and size given to the constructor
    max_force   = force*size*size;

    // speed and type of wheels are set
    this->speed = speed;
    this->sphereWheels = sphereWheels;

    height=size;
    length=size/2.5; // length of body
    width=size/2;  // diameter of body
    radius=size/6; // wheel radius
    wheelthickness=size/20; // thickness of the wheels (if wheels used, no spheres)
    cmass=8*size;  // mass of the body
    wmass=size;    // mass of the wheels
    sensorno=4;    // number of sensors
    motorno=4;     // number of motors
    segmentsno=5;  // number of segments of the robot

    wheelsubstance.toRubber(50);

  };


  /** sets actual motorcommands
      @param motors motors scaled to [-1,1]
      @param motornumber length of the motor array
  */
  void Nimm4::setMotorsIntern(const double* motors, int motornumber){
    assert(created); // robot must exist
    // the number of controlled motors is minimum of
    // __PLACEHOLDER_6__ (motornumber) and
    // __PLACEHOLDER_7__ (motorno)
    int len = (motornumber < motorno)? motornumber : motorno override;

    // for each motor the motorcommand (between -1 and 1) multiplied with speed
    // is set and the maximal force to realize this command are set
    for (int i=0; i<len; ++i) {
      joints[i]->setParam(dParamVel2, motors[i]*speed);
      joints[i]->setParam(dParamFMax2, max_force);
    }
  };

  /** returns actual sensorvalues
      @param sensors sensors scaled to [-1,1] (more or less)
      @param sensornumber length of the sensor array
      @return number of actually written sensors
  */
  int Nimm4::getSensorsIntern(sensor* sensors, int sensornumber){
    assert(created); // robot must exist

    // the number of sensors to read is the minimum of
    // __PLACEHOLDER_8__ (sensornumber) and
    // __PLACEHOLDER_9__ (sensorno)
    int len = (sensornumber < sensorno)? sensornumber : sensorno override;

    // for each sensor the anglerate of the joint is red and scaled with 1/speed
    for (int i=0; i<len; ++i) {
      sensors[i]=dynamic_cast<Hinge2Joint*>(joints[i])->getPosition2Rate();
      sensors[i]/=speed;  //scaling
    }
    // the number of red sensors is returned
    return len;
  };


  void Nimm4::placeIntern(const osg::Matrix& pose){
    // the position of the robot is the center of the body (without wheels)
    // to set the vehicle on the ground when the z component of the position is 0
    // width*0.6 is added (without this the wheels and half of the robot will be in the ground)
    Matrix p2;
    p2 = pose * Matrix::translate(Vec3(0, 0, width*0.6));
    create(p2);
  };


  /**
   * updates the osg notes
   */
  void Nimm4::update() {
    OdeRobot::update();
    assert(created); // robot must exist

    for (int i=0; i<segmentsno; ++i) { // update objects
      objects[i]->update();
    }
    for (int i=0; i < 4; ++i) { // update joints
      joints[i]->update();
    }

  };


  /** creates vehicle at desired pose
      @param pose matrix with desired position and orientation
  */
  void Nimm4::create( const osg::Matrix& pose ){
    if (created) {  // if robot exists destroy it
      destroy();
    }
    // create car space
    odeHandle.createNewSimpleSpace(parentspace, true);
    objects.resize(5);  // 1 capsule, 4 wheels
    joints.resize(4); // joints between cylinder and each wheel

    OdeHandle wheelHandle(odeHandle);
    // make the material of the wheels a hard rubber
    wheelHandle.substance = wheelsubstance;
    // create cylinder for main body
    // initialize it with ode-, osghandle and mass
