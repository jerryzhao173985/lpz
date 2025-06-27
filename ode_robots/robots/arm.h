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

#ifndef __ARM_H
#define __ARM_H

#include <selforg/inspectable.h>
#include <list>
#include <vector>
#include <cmath>

#include <cstring>
#include <iostream>

#include <selforg/matrix.h>
#include "oderobot.h"
// include primitives (box, spheres, cylinders ...)
#include "primitive.h"
#include "oneaxisservo.h"
#include "joint.h"
#include "sensor.h"


#include "osgprimitive.h"

using namespace matrix;

namespace lpzrobots{

  struct ArmConf
  {
    double body_mass = 1.0;
    double body_height = 5.0;
    double body_width = 2.0;
    double body_depth = 0.5;

    double shoulder_mass = 0.005;
    double shoulder_radius = 0.03;
    double joint_offset = 0.005; // distance of shoulder components from each other

    double upperarm_mass = 0.1;
    double upperarm_radius = 0.05;
    double upperarm_length = 1.5;

    double forearm_mass = 0.1;
    double forearm_radius = 0.05;
    double forearm_length = 1.2;

    double elevation_min = -M_PI/3;
    double elevation_max = M_PI/3;
    double humeral_min = -M_PI/4;
    double humeral_max = M_PI/4;
    double azimuthal_min = -M_PI/4;
    double azimuthal_max = M_PI/3;
    double elbow_min = -M_PI/3.5;
    double elbow_max = M_PI/3.5;

    double motorPower = 5;
    double damping = 0.2;    // motor damping
    double servoFactor = 1; // reduces servo angle constraints to servoFactor percent of hingeJoint angle constraints
    double scaleMotorElbow = 0.6;

    bool withContext = false; // if true context sensors are the effector positions
    bool useJointSensors = true; // if true joint sensors otherwise effector positions

    std::list<Sensor*> sensors; // list of additional sensors

  };

  class Arm : public OdeRobot, public Inspectable {
  public:

    /* Enumeration of different parts and joints */
    enum parts
      {
        base,
        shoulder1,
        shoulder2,
        upperArm,
        foreArm,
        hand
      };

    Arm(const OdeHandle& odeHandle, const OsgHandle& osgHandle, const ArmConf& conf, const std::string& name);

    static ArmConf getDefaultConf() {
      return ArmConf();
    }

    virtual ~Arm() {}

    virtual paramkey getName() const noexcept override {return "Arm";}

    /**
     * sets the pose of the vehicle
     * @param pose desired 4x4 pose matrix
     */
    virtual void placeIntern(const osg::Matrix& pose) override;

    /**
     * update the subcomponents
     */
    virtual void update() override;

    /**
     * returns actual sensorvalues
     * @param sensors sensors scaled to [-1,1]
     * @param sensornumber length of the sensor array
     * @return number of actually written sensors
     */
    virtual int getSensorsIntern(double* sensors, int sensornumber) override;

    /**
     * sets actual motorcommands
     * @param motors motors scaled to [-1,1]
     * @param motornumber length of the motor array
     */
    virtual void setMotorsIntern(const double* motors, int motornumber) override;

    /**
     * returns number of sensors
     */
    virtual int getSensorNumberIntern() const override {
      return sensorno;
    };

    /**
     * returns number of motors
     */
    virtual int getMotorNumberIntern() const override {
      return motorno;
    };

    /**
     * returns a vector with the positions of all segments of the robot
     * @param poslist vector of positions (of all robot segments)
     * @return length of the list
     */
    virtual int getSegmentsPosition(std::vector<Position> &poslist) const;

    /**
     * returns the position of the endeffector (hand)
     * @param position vector position vector
     */
    void getEndeffectorPosition(double* position) const;

    /**
     * this function is called in each timestep. It should perform robot-internal checks,
     * like space-internal collision detection, sensor resets/update etc.
     * @param globalData structure that contains global data from the simulation environment
     */
    virtual void doInternalStuff(const GlobalData& globalData) override;

    /******** CONFIGURABLE ***********/
    virtual void notifyOnChange(const paramkey& key) override;

    virtual Primitive* getMainObject() const {
      return objects[base];
    }

    /**
     * the main object of the robot, which is used for position and speed tracking
     */
    virtual Primitive* getMainPrimitive() const override {
      return objects[hand];
    }

    void setDlearnTargetHack(double* post);
    void setDmotorTargetHack(double* post);

  protected:

    /**
     * creates vehicle at desired pose
     * @param pose 4x4 pose matrix
     * @param snowmanmode snowman body
     */
    virtual void create(const osg::Matrix& pose);

    /**
     * destroys vehicle and space
     */
    virtual void destroy();

    static void mycallback(void *data, dGeomID o1, dGeomID o2);

    void hitTarget();

    double dBodyGetPositionAll ( dBodyID basis , int para );
    double dGeomGetPositionAll ( dGeomID basis , int para );

    void BodyCreate(int n, dMass m, dReal x, dReal y, dReal z, dReal qx, dReal qy, dReal qz, dReal qangle);

    // inspectable interface
    //virtual std::list<Inspectable::iparamkey> getInternalParamNames() const override;
    //virtual std::list<Inspectable::iparamval> getInternalParams() const override;
    //                virtual std::list<ILayer> getStructuralLayers() const override;
    //                virtual std::list<IConnection> getStructuralConnections() const override;


    ArmConf conf;
    matrix::Matrix endeff;

    paramval factorSensors;
    paramval print;



    std::vector <HingeServo*> hingeServos;

    int sensorno = 0;      // number of sensors
    int motorno = 0;       // number of motors

    bool created = false;      // true if robot was created

    // dSpaceID parentspace; // already defined in OdeRobot parent class

    int printed = 0;

  };
}
#endif
