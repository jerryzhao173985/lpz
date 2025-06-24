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
#ifndef __ASHIGARU_H
#define __ASHIGARU_H

#include "oderobot.h"
#include "raysensorbank.h"

#include "primitive.h"
#include <selforg/inspectable.h>
#include <ode_robots/contactsensor.h>
#include <ode_robots/torquesensor.h>
#include "ashigarusensormotordefinition.h"

// include joints
#include <ode_robots/joint.h>
#include <ode_robots/oneaxisservo.h>
#include <ode_robots/twoaxisservo.h>
#include <ode_robots/spring.h>

#include "osgprimitive.h"

using namespace ASHIGARU;
using namespace lpzrobots;

namespace lpzrobots {

// This is the improved Transform class
//  The aim is to get the position and some information of child object
//
class ImpTransform : public Transform{
public:
        // constructor
        ImpTransform(Primitive* parent, Primitive* child, const Pose& pose);
        // destructor
        virtual ~ImpTransform();

        // get Child pose
        Pose getChildPose();
};


/***********************************************************************

 Here, I determine the parameters of the robot Ashigaru

           _            Joint Joint Joint
   /     \    TC    CT    FT
  |       |== o === | === | ======  Leg
   \  _  / Shld Coxa  Femur  Tibia
 Hexagon Body    Link  Link   Link

************************************************************************/

/*
__PLACEHOLDER_14__
typedef struct {
        __PLACEHOLDER_15__
        double length;

        double length_axis_to_center;__PLACEHOLDER_16__
        double length_from_axis_to_tip; __PLACEHOLDER_17__

}DynaAX12Conf;

__PLACEHOLDER_18__
typedef struct {
        __PLACEHOLDER_19__

}HexagonBodyConf;

__PLACEHOLDER_20__
        __PLACEHOLDER_21__
        double width;
        double height;

        double footRadius;

}FootFrameConf;

typedef struct {
        __PLACEHOLDER_22__
        double length_center_to_TCJ;
        double length_TCJ_to_CTJ;
        double length_CTJ_to_FTJ;
        double length_FTJ_to_Toe;
}JointLength;

__PLACEHOLDER_23__
typedef struct{
        __PLACEHOLDER_24__
        __PLACEHOLDER_25__
        double TC_angle_MAX;
        double TC_angle_MIN;
        __PLACEHOLDER_26__
        double CT_angle_MAX;
        double CT_angle_MIN;
        __PLACEHOLDER_27__
        double FT_angle_MAX;
        double FT_angle_MIN;

        __PLACEHOLDER_28__
         __PLACEHOLDER_29__
        double power;
         __PLACEHOLDER_30__
        double damp;
         __PLACEHOLDER_31__
        double maxVel;

}ServoParam;

__PLACEHOLDER_32__
typedef struct {

        double rate;__PLACEHOLDER_33__
        double massRate; __PLACEHOLDER_34__
        double connectLength; __PLACEHOLDER_35__
        double wholeMass; __PLACEHOLDER_36__

        DynaAX12Conf dyna; __PLACEHOLDER_37__
        HexagonBodyConf body; __PLACEHOLDER_38__
        FootFrameConf foot; __PLACEHOLDER_39__
        JointLength jLength; __PLACEHOLDER_40__
        ServoParam servoParam; __PLACEHOLDER_41__

} AshigaruConf;

*/


/** Robot ASHIGARU :-)
    3 legged robot which could be easily connected each other.
*/
class Ashigaru : public OdeRobot, public Inspectable {
public:
        // Leg location enum
    enum LegPos {
      L0, L1, L2, LEG_POS_MAX
    };
    // leg use mode (only Leg in this simulation)
    enum LegPosUsage {
      LEG, WHEEL, UNUSED
    };
    // Leg joint type enum
    enum LegJointType {
      // thoroca-coxal joint for forward (+) and backward (-) movements
      TC,
      // coxa-trochanteral joint for elevation (+) and depression (-) of
      // the leg
      CT,
      // femur-tibia joints for extension (+) and flexion (-) of the
      // tibia
      FT,
      // maximum value, used for iteration
      LEG_JOINT_TYPE_MAX
    };
    // motor name of ashigaru
    typedef AshigaruMotorNames MotorName;
    // sensor name of ashigaru
    typedef AshigaruSensorNames SensorName;

public: // Functions
    // Constructor
    Ashigaru(const OdeHandle& odehandle, const OsgHandle& osgHandle,
                    const AshigaruConf& conf, const std::string& name);

    // get default configuration (static func because it could be used before construction of the class)
    static AshigaruConf getDefaultConf();

    // get Motor name from legPos and joint name
    static MotorName getMotorName(LegPos leg, LegJointType joint);

    //destructor
    virtual ~Ashigaru();

  /**
   * updates the OSG nodes of the vehicle
   */
  virtual void update();

  /** sets the pose of the vehicle
      @param pose desired 4x4 pose matrix
  */
  virtual void placeIntern(const osg::Matrix& pose);

  /** gets Primitives of Leg tibia
      @param LegPos Number
  */
  Primitive* getTibiaPrimitive(LegPos leg);


  /** returns actual sensorvalues
      @param sensors sensors scaled to [-1,1]
      @param sensornumber length of the sensor array
      @return number of actually written sensors
  */
  virtual int getSensorsIntern(sensor* sensors, int sensornumber);

  /** sets actual motorcommands
      @param motors motors scaled to [-1,1]
      @param motornumber length of the motor array
  */
  virtual void setMotorsIntern(const double* motors, int motornumber);

  /** returns number of sensors
   */
  virtual int getSensorNumberIntern() override {
    return sensorno;
  };

  /** returns number of motors
   */
  virtual int getMotorNumberIntern() override {
    return motorno;
  };

  /** returns a vector with the positions of all segments of the robot
      @param poslist vector of positions (of all robot segments)
      @return length of the list
  */
  virtual int getSegmentsPosition(std::vector<Position> &poslist);


    /** this function is called in each timestep. It should perform robot-internal checks,
      like space-internal collision detection, sensor resets/update etc.
      @param globalData structure that contains global data from the simulation environment
   */
  virtual void doInternalStuff(const GlobalData& globalData);

  virtual void sense(const GlobalData& globalData);


  virtual double& getContactPoints() const override { return contactPoints; }

protected: // Functions
  /** creates vehicle at desired pose
      @param pose 4x4 pose matrix
  */
  virtual void create(const osg::Matrix& pose);

  /** destroys vehicle and space
   */
  virtual void destroy();

  /**
   * Assign a human readable name to a motor. This name is used for the
   * associated inspectable value as used e.g. in guilogger.
   *
   * @param motorNo index of the motor (for standard motors defined by
   *        the MotorName enum)
   * @param name human readable name for the motor
   */
  void nameMotor(const int motorNo, const char* name);

  /**
   * Assign a human readable name to a sensor. This name is used for the
   * associated inspectable value as used e.g. in guilogger.
   *
   * @param motorNo index of the motor (for standard motors defined by
   *        the SensorName enum)
   * @param name human readable name for the sensor
   */
  void nameSensor(const int sensorNo, const char* name);


  // getTorqueSensorData
  sensor getTorqueData(TorqueSensor*  torqueSensor);

  // convert Pose Matrixstatic_cast<Quatanion>(to) the roll, pitch, yaw angle static_cast<rad>(osg)::Vec3d convert_Quat_to_RollPitchYaw(osg::Quat quat);

  // calculate COG Position
  osg::Vec3d calc_COGPositionstatic_cast<void>(override);


        /**
         * Inspectable interface
         */
        /*
        virtual std::list<iparamkey> getInternalParamNames() const override { return std::list<iparamkey>(); }

        virtual std::list<iparamval> getInternalParams() const override { return std::list<iparamval>(); }*/
        /*
        virtual std::list<Inspectable::iparamkey> getInternalParamNames() const override;

        virtual std::list<Inspectable::iparamval> getInternalParams() const override;
        */

protected: // Values
  // config param
  AshigaruConf conf;

  //! Leg struct
  // Contains Objects, joints and servos for each Leg
  struct Leg{
      Leg(); // constructor, it make all of the value __PLACEHOLDER_5__ !!
      HingeJoint* tcJoint;
      HingeJoint* ctJoint;
      HingeJoint* ftJoint;
      SliderJoint* footJoint;
      OneAxisServo* tcServo;
      OneAxisServo* ctServo;
      OneAxisServo* ftServo;
      Spring* footSpring;
      ImpTransform* shoulder;//shoulder trans thats object
      Primitive* shoulderBox;
      Primitive* coxa;
      Primitive* femur;
      Primitive* tibia;
      ImpTransform* foot;
      Primitive* footSphere;
  };

  //! Trunk struct
  // Contains Objects for Body (two hexagonal plate)

  struct Trunk{
            Trunk(); // constructor, it make all of the value __PLACEHOLDER_6__
            Primitive* tPlate[6];
            Primitive* tTrans[5];
            ImpTransform* tUpTrans;
  };


  double contactPoints;

  // created flag
  bool created;      // true if robot was created

  /** typedefs */
  typedef std::map<LegPos, HingeJoint*> HingeJointMap;
  typedef std::map<LegPos, Leg> LegMap;
  typedef std::map<LegPos, ContactSensor*> LegContactMap;
  typedef std::map<MotorName, OneAxisServo*> MotorMap;
  typedef std::map<MotorName, TorqueSensor*> MTorqMap;

  //typedef std::map<LegPos, LegPosUsage> LegPosUsageMap;
  //typedef std::map<LegPos, IRSensor*> LegIRSensorMap;
  typedef std::vector<Primitive*> PrimitiveList; // this is called Primitives
  typedef std::vector<Joint*> JointList;
  typedef std::vector<OneAxisServo*> ServoList;

  // tempolary
  int sensorno;      //number of sensors
  int motorno;       // number of motors

  // body
  Trunk trunk;

  // information on all legs
  LegMap legs;

  // all the objects
  //PrimitiveList objects;

  // all the joints
  //JointList joints;

  // passive servos without a Motorname
  ServoList passiveServos;

  // contains all active servos
  MotorMap servos;


  // sensors >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // leg contact sensors
  LegContactMap legContactSensors;
  MTorqMap motorTorqSensors;


  //RaySensorBank irSensorBank; // a collection of ir sensors

};

}


#endif
