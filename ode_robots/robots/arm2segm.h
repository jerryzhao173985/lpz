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
#ifndef __ARM2SEGM_H
#define __ARM2SEGM_H

#include "oderobot.h"
#include <selforg/configurable.h>
#include <vector>

#include "primitive.h"
#include "joint.h"

#include "angularmotor.h"
namespace lpzrobots{

  struct Arm2SegmConf {



    double max_force = 5;       // maximal force for motors
    int segmentsno = 4;          // number of segments
    double base_mass = 0.5;      // mass of base segment
    double base_length = 0.4;    // length of base segment
    double base_width = 0.1;     // width of base segment
    double arm_mass = 0.1;       // mass of arm elements
    double arm_width = 0.2;      // width static_cast<thickness>(of) arms
    double arm_length = 1.2;     // length of arms
    double arm_offset = 0.03;    // offset between arms (so that they do not touch)
    double joint_offset = 0.2;   // overlapping of arms (to have area for joints)
  };



  class Arm2Segm : public OdeRobot {
  public:

    Arm2Segm(const OdeHandle& odeHandle, const OsgHandle& osgHandle, const Arm2SegmConf& conf, const std::string& name);

    virtual ~Arm2Segm() {};

    static Arm2SegmConf getDefaultConf() {
      return Arm2SegmConf();
    }

    /// update the subcomponents
    virtual void update() override;

    /** sets the pose of the vehicle
        @param pose desired 4x4 pose matrix
    */
    virtual void placeIntern(const osg::Matrix& pose) override;

    /** returns actual sensorvalues
        @param sensors sensors scaled to [-1,1]
        @param sensornumber length of the sensor array
        @return number of actually written sensors
    */
    virtual int getSensorsIntern(double* sensors, int sensornumber) override;

    /** sets actual motorcommands
        @param motors motors scaled to [-1,1]
        @param motornumber length of the motor array
    */
    virtual void setMotorsIntern(const double* motors, int motornumber) override;

    /** returns number of sensors
     */
    virtual int getSensorNumberIntern() const override {
      return sensorno;
    };

    /** returns number of motors
     */
    virtual int getMotorNumberIntern() const override {
      return motorno;
    };

    /** returns a vector with the positions of all segments of the robot
        @param poslist vector of positions (of all robot segments)
        @return length of the list
    */
    virtual int getSegmentsPosition(std::vector<Position> &poslist);

    /** the main object of the robot, which is used for position and speed tracking */
    virtual const Primitive* getMainPrimitive() const override;

  protected:

    /** creates vehicle at desired pose
        @param pose 4x4 pose matrix
    */
    virtual void create(const osg::Matrix& pose);

    /** destroys vehicle and space
     */
    virtual void destroy();

    Arm2SegmConf conf;



    std::vector <AngularMotor1Axis*> amotors;

    std::string name;
    paramval speed;
    paramval factorSensors;

    int sensorno = 0;      //number of sensors
    int motorno = 0;       // number of motors

    bool created = false;      // true if robot was created
  };
};
#endif
